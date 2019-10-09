#!/usr/bin/env python3
#
# Generate a C header file with Doxygen annotations from a Kconfig
# specification.
#
# Copyright (C) 2020 HAW Hamburg
#

import argparse
import os
import re
import sys
import textwrap
import kconfiglib

IGNORED_PREFIXES = ['KCONFIG']

def print_header(out, ingroup):
    """Prints the doxygen header for the header file.

    Args:
        out: Output file to write
        ingroup: Name of the Doxygen group to be part of
    """
    header = ('/**\n'
              ' * @ingroup {}\n'
              ' * @{{\n'
              ' */\n')
    header = header.format(ingroup)
    out.write(header)

def print_footer(out):
    """Prints the Doxygen footer for the header file.

    Args:
        out: Output file to write
    """
    out.write('/** @} */\n')

def should_generate_macro(node, state):
    """Determines if a macro should be generated for a node

    Args:
        node: Node to evaluate
        state: Global state of the symbols
    """
    sym = node.item
    if isinstance(sym, kconfiglib.Symbol) and sym.type != 'UNKNOWN' and sym.nodes:
        prefix = sym.name.split('_')[0]
        return prefix not in IGNORED_PREFIXES and sym.name not in state and node.prompt
    return False

def format_expression(expr, skip_y=True, prefix= " if "):
    expr_str = kconfiglib.expr_str(expr)
    if expr_str == '"y"' and skip_y:
        return ''
    return prefix + re.sub(r'\b([A-Z][A-Z0-9_]*)\b', r'@ref \1', expr_str)


def generate_macro_type(item):
    """Generates a macro doc line that contains the option type

    Args:
        node: Node from which to generate the type
    """
    lines = []
    lines += ' *  - **Type:** `{}`\n'.format(kconfiglib.TYPE_TO_STR[item.orig_type])
    return lines

def generate_macro_ranges(item):
    ranges = item.ranges
    lines = []
    if ranges:
        lines += ' *  - **Range{}:**\n'.format('s' if len(ranges) > 1 else '')
        for r in ranges:
            cond = format_expression(r[2])
            lines += ' *    - `{0} - {1}{2}`\n'.format(r[0].name, r[1].name, cond)
    return lines

def generate_macro_help(node):
    lines = []
    if node.help:
        lines = node.help.split('\n')
        lines = ['\n * ' + l for l in lines]
        lines[0] = lines[0][1:] + '\n'
    return lines

def generate_dependencies(node):
    lines = []
    has_deps = False
    if isinstance(node.dep, tuple):
        for d in node.dep:
            if isinstance(d, kconfiglib.Symbol):
                has_deps = True
                break
    if has_deps:
        lines += ' *  - **Depends on:**\n'
        for d in node.dep:
            if isinstance(d, kconfiglib.Symbol):
                lines += ' *    - `{0}`\n'.format(d.name)
    return lines

def generate_macro_defaults(item):
    defaults = item.defaults
    lines = []
    if defaults:
        if len(defaults) == 1:
            value = format_expression(defaults[0][0], skip_y=False, prefix="")
            condition = format_expression(defaults[0][1])
            lines += ' *  - **Default:** {0}{1}\n'.format(value, condition)
        else:
            lines += ' *  - **Defaults: **\n'
            for d in defaults:
                value = format_expression(d[0], skip_y=False, prefix="")
                condition = format_expression(d[1])
                lines += ' *    - {0}{1}\n'.format(value, condition)
    return lines

def generate_macro(node):
    """Generates a macro for the given node and returns the string

    Args:
        node: Note from which to generate the macro

    Returns:
        The an array of strings, one element for each line, of the generated
        macro.
    """
    sym = node.item
    lines = ['/**\n']
    lines += ' * @brief {}\n *\n'.format(node.prompt[0])
    lines += generate_macro_type(node.item)
    lines += generate_macro_ranges(node.item)
    lines += generate_macro_defaults(node.item)
    lines += generate_dependencies(node)
    lines += ' *\n'
    lines += generate_macro_help(node)
    lines += ' */\n'
    lines += '#define CONFIG_{}\n'.format(sym.name)
    return lines

def write_node(out, node, state):
    """Writes the content a node and all the nested ones intou the output file
    
    Args:
        out: Output file to write
        node: Current node to write
        state: Global state of the symbols
    """
    if should_generate_macro(node, state):
        macro = generate_macro(node)
        for l in macro:
            out.write(l)

    if node.list:
        entry = node.list
        while entry:
            write_node(out, entry, state)
            entry = entry.next

def main():
    """Entry point"""
    parser = argparse.ArgumentParser(
        description=('Generate a C header file with Doxygen annotations from a'
                     ' Kconfig specification.'))
    parser.add_argument('-k', '--kconfig', default='Kconfig',
        help='Kconfig file to read')
    parser.add_argument('-o', '--output', required=True,
        type=argparse.FileType('w'), help='Output file')
    parser.add_argument('-g', '--group', default='kconfig',
        help='Doxygen group to be in')
    args = parser.parse_args()

    parts = os.path.split(args.kconfig)
    oldwd = os.getcwd()
    os.chdir(parts[0])
    kconfig = kconfiglib.Kconfig(parts[1])
    os.chdir(oldwd)

    # Keeps track of the state for the printed symbols
    state = {}

    print_header(args.output, args.group)
    write_node(args.output, kconfig.top_node, state)
    print_footer(args.output)

    args.output.close()

if __name__=='__main__':
    main()
