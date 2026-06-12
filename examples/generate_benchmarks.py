#!/usr/bin/env python3
import csv
import random

def generate_large():
    print("Generating large.csv (100,000 rows, 10 columns)...")
    with open("large.csv", "w", newline="") as f:
        writer = csv.writer(f)
        for _ in range(100000):
            writer.writerow(["cell" + str(random.randint(0, 1000)) for _ in range(10)])

def generate_wide():
    print("Generating wide.csv (1,000 rows, 1,000 columns)...")
    with open("wide.csv", "w", newline="") as f:
        writer = csv.writer(f)
        for _ in range(1000):
            writer.writerow(["cell" + str(random.randint(0, 100)) for _ in range(1000)])

def generate_quoted():
    print("Generating quoted.csv (100,000 rows, 10 columns with quotes and newlines)...")
    with open("quoted.csv", "w", newline="") as f:
        writer = csv.writer(f)
        for _ in range(100000):
            writer.writerow(["\"quoted, string\n" + str(random.randint(0, 1000)) + "\"" for _ in range(10)])

if __name__ == "__main__":
    generate_large()
    generate_wide()
    generate_quoted()
    print("Done!")
