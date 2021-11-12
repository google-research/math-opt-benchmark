import csv
import json

items = {}
with open('orders.csv', 'r') as f:
    reader = csv.reader(f)
    for row in reader:
        for item in row:
            if item not in items:
                items[item] = 0
            items[item] += 1

print("Writing")

items_arr = [items[i] for i in sorted(items.keys(), key=int)]

with open('frqs.json', 'w') as f:
    frqs = json.dump(items_arr, f)
    # f.write(str(len(items_arr)) + ' ')
    # f.write(frqs)