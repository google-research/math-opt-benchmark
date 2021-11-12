import json
import dataset_pb2
import random
from collections import defaultdict

random.seed(4224)

print("Loading data", end='\r')

with open('frqs.json', 'r') as f:
    # Fix off-by-one indexing
    frqs = [-1] + json.load(f)

with open('aisles.json', 'r') as f:
    aisles_ = json.load(f)
    aisles = {}
    for key in aisles_:
        aisles[int(key)] = list(map(int, aisles_[key]))

def get_aisle(idx):
    for aisle, aisle_items in aisles.items():
        if idx in aisle_items:
            return aisle

items_ = [None]
for i in range(1, len(frqs)):
    item = dataset_pb2.Item()
    item.index = i
    item.freq = frqs[i]
    item.aisle = get_aisle(i)
    items_.append(item)

items = sorted(items_[1:], key=lambda x: x.freq, reverse=True)
aisles = defaultdict(list)
aisle_freqs = {}
for item in items:
    aisles[item.aisle].append(item.index)
    # Cumulative
    if item.aisle not in aisle_freqs:
        aisle_freqs[item.aisle] = [item.freq]
    else:
        aisle_freqs[item.aisle].append(item.freq + aisle_freqs[item.aisle][-1])


with open('orders.json', 'r') as f:
    orders_ = json.load(f)
    orders = list(orders_.values())
    for i in range(len(orders)):
        orders[i] = list(map(int, orders[i]))
    random.shuffle(orders)

small_orders = []
medium_orders = []
large_orders = []

order_set = [small_orders, medium_orders, large_orders]
order_sizes = [10, 25, 50]

sub_probability = 1/3
dataset_size = 10
num_substitutions = 3
for i in range(dataset_size):
    print('|' + ('*'*i).ljust(10, '-') + '|', end='\r')
    for j, order_size in enumerate(order_sizes):
        cart_orders = orders[:order_size]
        order_dataset = dataset_pb2.OrderDataset()
        for _, order in enumerate(cart_orders):
            order_proto = order_dataset.orders.add()
            for item in order:
                # Ignore some dataset inconsistencies (could be a parsing error somewhere)
                if item >= len(items):
                    continue
                order_proto.items.append(item)
                if random.random() < sub_probability:
                    sub = dataset_pb2.Substitution()
                    sub.index = item
                    subset = set()
                    aisle = items_[item].aisle
                    while len(subset) < 3:
                        s = random.choices(aisles[aisle], cum_weights=aisle_freqs[aisle], k=3)
                        subset = subset.union(s)
                    sub.sub_idxs.extend(subset)
                    # order_proto.subs.append(subs[item])

        # Remove unused subs
        item_set = set()
        for order in order_dataset.orders:
            for item in order.items:
                item_set.add(item)
        for order in order_dataset.orders:
            for k, sub in enumerate(order.subs):
                new_sub = [idx for idx in sub.sub_idxs if idx in item_set]
                if len(new_sub):
                    order.subs[k] = new_sub

        # Normalize indices
        ctr = -1
        def increment():
            global ctr
            ctr += 1
            return ctr
        new_idxs = defaultdict(increment)
        for order in order_dataset.orders:
            for k, item in enumerate(order.items):
                order.items[k] = new_idxs[item]
            for sub in order.subs:
                sub.index = new_idxs[sub.index]
                for h, item2 in enumerate(sub.sub_idxs):
                    sub.sub_idxs[h] = new_idxs[item2]
        order_dataset.nItems = len(item_set)
        order_set[j].append(order_dataset)
    random.shuffle(orders)

for i, order_dataset in enumerate(order_set):
    for j, order_proto in enumerate(order_dataset):
        with open(f'dataset/orders{10*i+j}.data', 'wb') as f:
            # f.write(str(order_proto))
            f.write(order_proto.SerializeToString())
