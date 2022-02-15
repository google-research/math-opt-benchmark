import os

base_dir = ""
home = "ORLIB/ORLIB-uncap/"
files = []
for dir in os.listdir(os.path.join(base_dir, home))[::-1]:
    next_dir = os.path.join(home, dir)
    if os.path.isdir(os.path.join(me, next_dir)):
        with open(os.path.join(me, next_dir,'files.lst'), 'r') as f:
            files += [os.path.join(next_dir, name) for name in f.readlines()]

with open('filepaths.txt', 'w') as f:
    for file in files:
        if file[-1] != '\n':
            file += '\n'
        f.write(file)
