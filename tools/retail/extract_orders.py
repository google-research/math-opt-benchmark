import csv
import json

orders = {}
with open('instacart-market-basket-analysis/order_products__prior.csv', 'r') as f:
    reader = csv.reader(f)
    keys = next(reader)
    for row in reader:
        if row[0] not in orders:
            orders[row[0]] = []
        orders[row[0]].append(row[1])

with open('orders.json', 'w') as f:
    json.dump(orders, f)

aisles = {}
with open('instacart-market-basket-analysis/products.csv', 'r') as f:
    reader = csv.reader(f)
    keys = next(reader)
    for row in reader:
        if row[2] not in aisles:
            aisles[row[2]] = []
        aisles[row[2]].append(row[0])


with open('aisles.json', 'w') as f:
    json.dump(aisles, f)


items = {}
for order in orders:
    for item in order:
        if item not in items:
            items[item] = 0
        items[item] += 1

items_arr = [items[i] for i in sorted(items.keys(), key=int)]

with open('frqs.json', 'w') as f:
    json.dump(items_arr, f)