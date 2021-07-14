#!/usr/bin/python3

import subprocess
from os import path, environ
import matplotlib.pyplot as plt
import matplotlib as mpl
import json
import numpy as np
import argparse
import logging
import yaml
import textwrap

DEFAULT_BOARDS = [
    "samr21-xpro"
]

DEFAULT_PROFILES = {
    "normal": {
        "config": "",
        "label": "normal"
    }
}

DEFAULT_MODULE_GROUPS = {
    "RIOT OS": {
        "modules": [
            ["core"],
            ["boards"],
            ["cpu"],
            ["drivers"],
            ["sys"],
        ]
    },
    "Packages": {
        "modules": [
            ["pkg"]
        ]
    }
}

MAKE = environ.get("MAKE", "make")

BASE_HATCHES = ['/', '\\', '|', '-', '+', 'x', '.']

CONFIG = {
    'boards': DEFAULT_BOARDS,
    'profiles': DEFAULT_PROFILES,
    'groups': DEFAULT_MODULE_GROUPS,
    'cache_dir': "/tmp",
    'count_ram': False
}

#the width of one column line
LATEX_COLUM_WIDTH_PT = 241.02039
 
#the width of both column lines with the gap in between
LATEX_TEXT_WIDTH_PT = 505.89
 
LATEX_FIG_HEIGHT_PT = 250
LATEX_FIG_HEIGHT_PT_SMALL = 180
 
def pt2inch(pt):
    """ Convert `pt` points to inches."""
    return pt * 0.0138889

def get_hatch(index):
    base = index % len(BASE_HATCHES)
    h = ''
    for i in range(index // len(BASE_HATCHES) + 1):
        h += BASE_HATCHES[base + i] * 2
    return h

def get_cosy_export_file_path(board, profile):
    profile = profile.strip().lower().replace(" ", "-")
    return path.join(CONFIG['cache_dir'], "{}_{}.json".format(board,profile))

def compile_for_board_and_profile(board, profile):
    env = environ.copy()
    cosy_export_path = get_cosy_export_file_path(board, profile)
    env["BOARD"] = board
    env["KCONFIG_ADD_CONFIG"] = CONFIG['profiles'][profile]['config']
    env["RIOT_CI_BUILD"] = "1"
    env["COSY_EXPORT"] = cosy_export_path

    p = subprocess.Popen((MAKE, 'clean'), env=env, stdout=subprocess.PIPE, stderr=subprocess.PIPE)
    res = p.wait()
    err = p.stderr.read()
    if res != 0:
        logging.error("Could not clean the application")
        raise RuntimeError(err)

    p = subprocess.Popen((MAKE, 'cosy-dump'),env=env, stdout=subprocess.PIPE, stderr=subprocess.PIPE)
    res = p.wait()
    err = p.stderr.read()
    if res != 0:
        logging.error("Could not build the application")
        raise RuntimeError(err)

    logging.info("Results exported to {}".format(cosy_export_path))
    return cosy_export_path

def aggregate_group_sizes(group, dump):
    flash_size = 0
    ram_size = 0

    for path in CONFIG['groups'][group]['modules']:
        value = dump
        sign = 1
        for p in path:
            if not value:
                break

            if p == "~":
                sign = -1
            else:
                value = value.get(p)
        if value:
                ram_size += sign * (value["size"]["b"] + value["size"]["d"])
                flash_size += sign * (value["size"]["t"] + value["size"]["d"])

    return (flash_size, ram_size)

def plot_for_board(results, export_path=None):
    # sizes is a dictionary where the keys are the module groups and the values
    # are maps containing arrays containing the sizes of the corresponding modules, one per configuration
    sizes = {}
    for group in CONFIG['groups'].keys():
        sizes[group] = {'flash': [], 'ram': []}

    # one value on x axis per run configuration
    runs = []
    for run in results['runs']:
        w = get_plot_configuration('label/width', 10)
        runs.append(textwrap.fill(run['profile']['label'], width=w))

        # open cosy dump
        dump = {}
        with open(run['dump_path'], "r") as d:
            dump = json.load(d)

        for group in sizes.keys():
            (flash, ram) = aggregate_group_sizes(group, dump)
            sizes[group]['flash'].append(flash)
            sizes[group]['ram'].append(ram)

    width = 0.7

    plt.style.use('styles.mplstyle')

    fig, [flash_ax, ram_ax] = plt.subplots(nrows=1, ncols=2,
                                           figsize=(pt2inch(3*LATEX_COLUM_WIDTH_PT),
                                                    pt2inch(1.6*LATEX_FIG_HEIGHT_PT)))

    plt.subplots_adjust(wspace=0.25, hspace=None)
    flash_ax.grid(linestyle='-.', linewidth=1, color='black', zorder=0)
    ram_ax.grid(linestyle='-.', linewidth=1, color='black', zorder=0)

    axs = {'flash': flash_ax, 'ram': ram_ax}

    accumulated = {
        'flash': np.zeros(len(runs)),
        'ram': np.zeros(len(runs))
    }

    max_value = {
        'flash': 0,
        'ram': 0
    }

    colors = plt.get_cmap("Set3").colors
    i = 0

    for (group, size) in sizes.items():
        logging.info("{}:{}".format(group, size))

        # use KiB
        values = {
            'flash': np.divide(size['flash'], 1024),
            'ram': np.divide(size['ram'], 1024)
        }

        # load group plot configuration
        hatch = get_group_configuration(group, 'hatch', get_hatch(i))
        color = get_group_configuration(group, 'color', colors[0])
        edge_color = get_group_configuration(group, 'edge-color', '#333')

        # add new bars
        axs['flash'].bar(runs, values['flash'], width, bottom=accumulated['flash'],
                         edgecolor=edge_color, label=group, color=color, hatch=hatch, zorder=2)
        axs['ram'].bar(runs, values['ram'], width, bottom=accumulated['ram'], edgecolor=edge_color,
                       label=group, color=color, hatch=hatch, zorder=2)

        # update the maximum and the accumulated array
        accumulated['flash'] = np.add(accumulated['flash'], values['flash']).tolist()
        accumulated['ram'] = np.add(accumulated['ram'], values['ram']).tolist()
        max_value['flash'] = np.max([np.max(accumulated['flash']), max_value['flash']])
        max_value['ram'] = np.max([np.max(accumulated['ram']), max_value['ram']])

        #remove last used color
        colors = colors[1:]
        i += 1

    # set maximum value for Y axis
    axs['flash'].set_ylim(0, max_value['flash'] * 1.1)
    axs['ram'].set_ylim(0, max_value['ram'] * 1.1)

    # set labels and legends
    axs['flash'].set_ylabel('FLASH [KiB]')
    axs['ram'].set_ylabel('RAM [KiB]')

    # put a legend below the axis
    fig.legend(sizes.keys(), ncol=1, bbox_to_anchor=(1.02, 0.5), loc='center', handletextpad=0.5)
    axs['flash'].set_xticklabels(runs, rotation=45, va='top', ha='right')
    axs['ram'].set_xticklabels(runs, rotation=45, va='top', ha='right')

    if export_path is not None:
        plt.savefig(export_path, bbox_inches='tight')
    else:
        plt.show()

def load_configuration(config_file=None):
    """ Loads configuration from a configuration file (config_file), and sets default values when
        not provided. The end result is set to the global variable CONFIG.
        
        - config_file is an open file descriptor
    """
    global CONFIG
    if config_file is not None:
        try:
            CONFIG.update(yaml.safe_load(config_file))
        except yaml.YAMLError as exc:
            logging.error("ERROR while reading configuration file. {}".format(exc))

def get_plot_configuration(path, default=None):
    """ Tries to get a value from the global plot configuration loaded from the configuration file.
        path specifies which value to get, where each hierarchy is separated by `/`.
        
        Example: to try to get the font size path would be `font/size`
    """
    global CONFIG
    v = CONFIG.get('plots',{}).get('config',{})

    for segment in path.split('/'):
        if v is not None:
            v = v.get(segment)
        else:
            break

    return v if v is not None else default

def get_group_configuration(group, path, default=None):
    """ Tries to get a value from the global configuration loaded from the configuration file.
        path specifies which value to get, where each hierarchy is separated by `/`.
        
        Example: to try to get the hatch the path would be `hatch`
    """
    global CONFIG
    v = CONFIG['groups'][group].get('config', {})

    for segment in path.split('/'):
        if v is not None:
            v = v.get(segment)
        else:
            break

    return v if v is not None else default

def main():
    # Define some command line args
    p = argparse.ArgumentParser(prog="RIOT dissector",
                                description="""Analyze FLASH and RAM footprint of your RIOT application
                                               with different configurations on multiple platforms.""",
                                epilog="To see what's really going on.")
    p.add_argument("--config", "-c", type=argparse.FileType('r'), default='config.yaml',
                   help="Configuration file containing grouping, boards and profiles")
    p.add_argument("--export-plot", "-e", type=str, help="File to which the plot is exported")
    p.add_argument("--debug", "-d", action="store_true", help="Show debug messages")
    p.add_argument("--force", "-f", action="store_true", help="Force compilation")
    args = p.parse_args()

    log_level = logging.INFO
    if args.debug:
        log_level = logging.DEBUG
    logging.basicConfig(encoding='utf-8', level=log_level)

    load_configuration(args.config)

    result = []
    for board in CONFIG['boards']:
        board_test = {}
        board_test['board'] = board
        board_test_runs = []

        for (profile, value) in CONFIG['profiles'].items():
            run = {}

            # use cache if possible
            cosy_file_path = get_cosy_export_file_path(board, profile)
            if args.force or not path.exists(cosy_file_path):
                logging.info("Rebuilding for board {} with profile {}".format(board, profile))
                run['dump_path'] = compile_for_board_and_profile(board, profile)
            else:
                logging.info("Using cache for board {} with profile {} at {}".format(board, profile, cosy_file_path))
                run['dump_path'] = cosy_file_path

            run['profile'] = value
            board_test_runs.append(run)

        board_test['runs'] = board_test_runs
        result.append(board_test)
        plot_for_board(board_test, args.export_plot)

if __name__ == "__main__":
    main()
