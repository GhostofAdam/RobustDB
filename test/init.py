import os
import sys
import pandas as pd

def main():
    dir = "./dataset_test"
    fr = open("./init.sql", "w")
    with open("./build.sql", "r", encoding="utf-8") as f:
        fr.writelines(f.readlines())
    fr.write("\n\n")
    for filename in list(os.listdir(dir)):
        table_name = filename.split(".")[0]
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
            fr.write("INSERT INTO {} VALUES ".format(table_name) + "(" + ",".join(["\'{}\'".format(x) for x in line]) + ");\n")
        fr.write("\n\n")
    fr.close()


if __name__ == "__main__":
    main()