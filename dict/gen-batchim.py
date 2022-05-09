#!/usr/bin/python

import os

# g2pK/utils.py
def parse_table():
    '''Parse the main rule table'''
    lines = open(os.path.join(os.path.split(os.path.realpath(__file__))[0], 'table.csv'), 'r', encoding='utf8').read().splitlines()
    onsets = lines[0].split(",")
    table = []
    for line in lines[1:]:
        cols = line.split(",")
        coda = cols[0]
        for i, onset in enumerate(onsets):
            cell = cols[i]
            if len(cell)==0: continue
            if i==0:
                continue
            else:
                str1 = f"{coda}{onset}"
                if "(" in cell:
                    str2 = cell.split("(")[0]
                    rule_ids = cell.split("(")[1][:-1].split("/")
                else:
                    str2 = cell
                    rule_ids = []

                table.append((str1, str2, rule_ids))
    return table

table = parse_table ()
lst = list()

for content in table:
    if "( ?)" in content[0]:
        lst.append (content[0].replace("( ?)","")
                    + "\t"
                    + content[1].replace("\\1",""))
        lst.append (content[0].replace("( ?)"," ")
                    + "\t"
                    + content[1].replace("\\1"," "))
    elif "(\\W|$)" in content[0]:
        lst.append (content[0].replace("(\\W|$)"," ")
                    + "\t"
                    + content[1].replace("\\1",""))
        lst.append (content[0].replace("(\\W|$)","$")
                    + "\t"
                    + content[1].replace("\\1","$"))
    else:
        lst.append (content[0]
                    + "\t"
                    + content[1])

result = open ("batchim.txt", "w", encoding="UTF-8")

result.write ("\n".join(lst))
