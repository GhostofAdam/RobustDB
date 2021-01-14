import os
import sys
import pandas as pd

mapping = {"part": [False, True, True, True, True, False, True, False, True],
           "region": [False, True, True],
           "nation": [False, True, False, True],
           "supplier": [False, True, True, False, True, False, True],
           "customer": [False, True, True, False, True, False, True, True],
           "partsupp": [False, False, False, False, True],
           "orders": [False, False, True, False, True, True, True, False, True],
           "lineitem": [False, False, False, False, False, False, False, False, True, True, True, True, True, True, True, True]}

def main():
    dir = "./dataset_test"
    fr = open("./init.sql", "w")
    with open("./build.sql", "r", encoding="utf-8") as f:
        fr.writelines(f.readlines())
    fr.write("\n\n")
    for filename in list(os.listdir(dir)):
        table_name = filename.split(".")[0]
        table_map = mapping[table_name]
        with open(os.path.join(dir, filename), "r", encoding="utf-8") as f:
            lines = f.readlines()
        if filename.endswith(".csv"):
            lines = lines[1:]
        elif filename.endswith(".tbl"):
            pass
        else:
            print(filename)
            raise NotImplementedError
        for line in lines:
            line = line.strip().split("|")
            if filename.endswith(".tbl"):
                line = line[:-1]
            fr.write("INSERT INTO {} VALUES ".format(table_name) + "(" + ",".join(["\'{}\'".format(x) if y else x for x, y in zip(line, table_map)]) + ");\n")
        fr.write("\n\n")
    fr.close()


if __name__ == "__main__":
    main()