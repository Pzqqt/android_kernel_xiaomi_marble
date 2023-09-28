#!/usr/bin/env python3
# encoding: utf-8

import os
import sys
import re
from typing import Optional, Final

from lxml import etree
from rich.console import Console
from rich.table import Table


__author__: Final = "Pzqqt"

LOCAL_DIR: Final = os.path.dirname(os.path.abspath(__file__))

def main(abi_gki_aarch64_xml_file: Optional[str] = None, module_symvers_file: Optional[str] = None) -> int:
    if abi_gki_aarch64_xml_file is None:
        abi_gki_aarch64_xml_file = os.path.join(LOCAL_DIR, 'android', 'abi_gki_aarch64.xml')
    if module_symvers_file is None:
        module_symvers_file = os.path.join(LOCAL_DIR, 'out', 'vmlinux.symvers')

    assert os.path.isfile(abi_gki_aarch64_xml_file)
    assert os.path.isfile(module_symvers_file)

    def _crc_value_fix(crc_value: str):
        assert isinstance(crc_value, str)
        if re.match(r'0x[0-9a-f]{8}', crc_value):
            return crc_value
        if re.match(r'0x[0-9a-f]{1,7}', crc_value):
            return '0x' + crc_value[2:].zfill(8)
        raise ValueError(crc_value)

    with open(abi_gki_aarch64_xml_file, 'r', encoding="utf-8") as f:
        elf_function_symbols = {
            elf_symbol.get("name"): _crc_value_fix(elf_symbol.get("crc"))
            for elf_symbol in etree.XML(f.read()).findall('.//elf-function-symbols/elf-symbol')
        }

    with open(module_symvers_file, 'r', encoding="utf-8") as f:
        module_symvers = {
            line.split()[1]: _crc_value_fix(line.split()[0]) for line in f.readlines()
        }

    if missing_symbols := elf_function_symbols.keys() - module_symvers.keys():
        print("Warning: The kernel image is missing the following symbols:")
        for symbol in missing_symbols:
            print('-', symbol)

    diff_crc_items = [
        (key, elf_function_symbols[key], module_symvers[key])
        for key in elf_function_symbols.keys() & module_symvers.keys()
        if elf_function_symbols[key] != module_symvers[key]
    ]

    if not diff_crc_items:
        print("No mismatching crc values found. Good job!")
        return 0

    rich_console = Console()

    rich_table = Table(show_header=True, header_style="bold magenta")
    rich_table.add_column("Function symbol", style="dim")
    rich_table.add_column("Crc from abi_gki_aarch64.xml")
    rich_table.add_column("Crc from vmlinux.symvers")
    for item in diff_crc_items:
        rich_table.add_row(*item)

    rich_console.print(rich_table)

    print("Found %d function symbol(s) with mismatched crc values!" % len(diff_crc_items))
    return 1

if __name__ == "__main__":
    if len(sys.argv) == 1:
        sys.exit(main())
    if len(sys.argv) == 3:
        sys.exit(main(sys.argv[1], sys.argv[2]))
    print('Usage: %s <abi_gki_aarch64.xml file> <vmlinux.symvers file>' % sys.argv[0])
    sys.exit(2)
