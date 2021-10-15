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

print("Writing")

with open('orders.json', 'w') as f:
    json.dump(orders, f)
    # writer = csv.writer(f, delimiter=',')
    # for order in orders:
    #     writer.writerow(orders[order])

print('Reading')

aisles = {}
with open('instacart-market-basket-analysis/products.csv', 'r') as f:
    reader = csv.reader(f)
    keys = next(reader)
    for row in reader:
        if row[2] not in aisles:
            aisles[row[2]] = []
        aisles[row[2]].append(row[0])


print("Writing")

with open('aisles.json', 'w') as f:
    json.dump(aisles, f)
    # writer = csv.writer(f, delimiter=',')
    # min_aisles = int(min(aisles.keys(), key=lambda x: int(x)))
    # num_aisles = int(max(aisles.keys(), key=lambda x: int(x)))
    # for i in range(min_aisles, num_aisles+1):
    #     writer.writerow(aisles[str(i)])
