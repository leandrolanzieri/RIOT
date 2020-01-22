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
import logging
import kconfiglib

IGNORED_SYMBOL_PREFIXES = ['KCONFIG']

class DocGroup(object):
    def __init__(self, groupName, fileName=None, fileBaseDir=None):
        self.name = groupName
        if not fileName:
            fileName = 'kconfig_{}.h'.format(groupName)
        if fileBaseDir:
            self.fileName = os.path.join(fileBaseDir, fileName)
        else:
            self.fileName = fileName

    def openFile(self, flags='w'):
        self.file = open(self.fileName, flags)

    def closeFile(self):
        self.file.close()

    def get_formatted_title(self):
        title = self.name.lower()
        if title.startswith('kconfig_'):
            title = title[len('kconfig_'):]
        title = title.replace('_', ' ') + ' compile configurations'
        return title.capitalize()

def print_header(out, group, groupTitle=None, ingroup=None):
    """Prints the doxygen header for the header file.

    Args:
        out: Output file to write
        ingroup: Name of the Doxygen group to be part of
    """
    if not groupTitle:
        groupTitle = '{} compile configurations'.format(group)

    header = ['/**']
    header.append(' * @defgroup {} {}'.format(group, groupTitle))

    if ingroup:
        header.append(' * @ingroup {}'.format(ingroup))

    header.append(' * @{')
    header.append(' */')
    for l in header:
        out.write(l + '\n')

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
        return prefix not in IGNORED_SYMBOL_PREFIXES and sym.name not in state and node.prompt
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

def write_node(node, state):
    """Writes the content a node and all the nested ones into the output file
    
    Args:
        out: Output file to write
        node: Current node to write
        state: Global state of the symbols
    """
    if node.list:
        if isinstance(node.item, kconfiglib.Symbol):
            enter_group(node.item.name.lower(), state)
        elif node.item == kconfiglib.MENU and node is not state['kconfig'].top_node:
            enter_group(node.prompt[0].replace(' ', '_').lower(), state)

    if should_generate_macro(node, state):
        macro = generate_macro(node)
        for l in macro:
            state['stack'][-1].file.write(l)

    if node.list:
        entry = node.list
        while entry:
            write_node(entry, state)
            entry = entry.next
        exit_group(state)

def generate_group_header(state, groupTitle=None):
    cur = state['stack'][-1]
    try:
        parent = state['stack'][-2]
    except:
        parent = None
    lines = ['/**']

    if not groupTitle:
        groupTitle = cur.get_formatted_title()

    lines.append(' * @defgroup {} {}'.format(cur.name, groupTitle))

    if parent:
        lines.append(' * @ingroup {}'.format(parent.name))
    lines.append(' * @{')
    lines.append(' */')
    return lines

def enter_group(group, state, groupTitle=None, fileName=None):
    """ Enters to a group of the given name.

        This also closes the file of the actual group and opens the one of the
        new one. If the group has already been created before if uses it and
        does not overwrite the file.

        Args:
            group: Group name
            state: Global state object
            fileName: Optional name for the file of the group
    """
    try:
        state['stack'][-1].closeFile()
    except:
        logging.debug('No parent group')

    # use existant group if present
    if group in state['groups']:
        g = state['groups'][group]
        g.openFile(flags='a')
        state['stack'].append(g)
    else:
        g = DocGroup(group, fileName=fileName, fileBaseDir=state['output_dir'])
        state['groups'][group] = g
        g.openFile()
        state['stack'].append(g)
        for l in generate_group_header(state, groupTitle=groupTitle):
            g.file.write(l + '\n')

def exit_group(state):
    """ Exits the current group, closes its file and opens the parent's one.

        Args:
            state: Global state object
    """
    try:
        state['stack'][-1].closeFile()
        del state['stack'][-1]
        state['stack'][-1].openFile(flags='a')
    except:
        logging.debug('No more groups')

def main():
    """Entry point"""
    parser = argparse.ArgumentParser(
        description=('Generate a C header file with Doxygen annotations from a'
                     ' Kconfig specification.'))
    parser.add_argument('-k', '--kconfig', default='Kconfig',
        help='Kconfig file to read')
    parser.add_argument('-o', '--outputDir', required=True,
        help='Output directory')
    parser.add_argument('-g', '--group', default='kconfig',
        help='Doxygen group to be in')
    parser.add_argument('-d', '--debug', dest='debug', action='store_true',
        help='Enable debug messages')
    args = parser.parse_args()

    parts = os.path.split(args.kconfig)
    oldwd = os.getcwd()
    os.chdir(parts[0])
    kconfig = kconfiglib.Kconfig(parts[1])
    os.chdir(oldwd)

    # Configure logging
    logLevel = logging.DEBUG if args.debug else logging.INFO
    logging.basicConfig(level=logLevel)

    state = {
        'groups': {}, # keeps track of created groups
        'stack': [],  # keeps track of the stack of nested groups
        'output_dir': args.outputDir, # output directory for files
        'kconfig': kconfig # Kconfig instance
    }

    # enter the root group
    enter_group('kconfig', state, groupTitle='Kconfig configuration options')

    write_node(kconfig.top_node, state)
    for g in state['groups'].values():
        g.openFile(flags='a')
        print_footer(g.file)
        g.closeFile()

if __name__=='__main__':
    main()