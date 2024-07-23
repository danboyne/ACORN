#!/usr/bin/python3

#
# This program reads an input text file and replaces text strings with replacement
# text strings. A mapping file contains the strings that are to be replaced, and
# the strings that will replace those strings. The program is invoked with three
# command-line arguments:
#    -i or --input: Specifies the path to the input text file.
#    -o or --output: Specifies the path to the output text file.
#    -m or --mapping: Specifies the path to the mapping file.
#
# The mapping file contains one line of text for each word to be replaced. The old 
# word and new word must be enclosed in single quotes and separated by a comma. 
# The first token (the old word) can be a regular expression. Leading and trailing 
# spaces are preserved within the single-quoted strings and regular expressions.
# Examples follow:
#
#    '^\s*ASLEEP\s','Net_0  '
#    ' SD1_\* line ','Line '
#    '\sUSB1_UID$',' Net_348'
#
#
# Disclosure: This program was created by ChatGPT 4o based on prompts by Dan Boyne
# on 19 July 2024.
#

import re        # This module is used for regular expressions.
import argparse  # This module is used to parse command-line arguments.

#
# Read the mapping file and creates a dictionary where the keys are the old words 
# and the values are the new words. The old word and new word must be enclosed in
# single quotes and separated by a comma. The first token (the old word) can be 
# a regular expression. Leading and trailing spaces are preserved within the 
# single-quoted strings and regular expressions.
#
def read_mapping_file(mapping_file_path):
    """Read the mapping file and return a list of tuples with compiled regex and replacement strings."""
    mappings = []
    with open(mapping_file_path, 'r') as mapping_file:
        for line in mapping_file:
            old_pattern, _, new_word = line.partition(',')
            old_pattern = old_pattern.strip().strip("'")  # Remove leading/trailing spaces and quotes
            new_word = new_word.strip().strip("'")  # Remove leading/trailing spaces and quotes
            mappings.append((re.compile(old_pattern), new_word))
    return mappings

#
# Read the input text file line by line, applies the regex replacements, 
# and writes the modified lines to a new output file.
#
def replace_words_in_text(input_file_path, output_file_path, mappings):
    """Read the input text file, replace words using regex mappings, and save the result to a new file."""
    with open(input_file_path, 'r') as input_file, open(output_file_path, 'w') as output_file:
        for line in input_file:
            for old_pattern, new_word in mappings:
                line = old_pattern.sub(new_word, line)
            output_file.write(line)


#
# This function sets up the argument parser and parses the command-line arguments:
#
def main():
    parser = argparse.ArgumentParser(description='Replace words in a text file based on a mapping file.')
    parser.add_argument('-i', '--input', required=True, help='Path to the input text file.')
    parser.add_argument('-o', '--output', required=True, help='Path to the output text file.')
    parser.add_argument('-m', '--mapping', required=True, help='Path to the mapping file.')

    args = parser.parse_args()

    # Read the mappings
    mappings = read_mapping_file(args.mapping)

    # Replace words in the input file and save to the output file
    replace_words_in_text(args.input, args.output, mappings)

    print(f"Processed text with regex replacements has been saved to {args.output}")

if __name__ == '__main__':
    main()


