import json
import argparse
import os
import glob


def drop_till_underscore(string):
    return string[string.index('_') + 1:]


def gen_map_entry(description):
    name = description["layer"]["name"]
    name = drop_till_underscore(name)
    name = drop_till_underscore(name)
    print("VKW_LAYER_MAP_ENTRY(" + name + ")")


if __name__ == '__main__':
    args = argparse.ArgumentParser()

    args.add_argument('-path', action='store', default='.',
                      help='path to Vulkan SDK')

    parsed_args = args.parse_args()

    os.chdir(parsed_args.path + "/Bin")

    for filename in glob.glob("*.json"):
        file = open(filename, 'r')
        gen_map_entry(json.load(file))
