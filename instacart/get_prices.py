import requests
from bs4 import BeautifulSoup
import csv
import re
import time
import pickle

HEADERS = {'User-Agent':
            'Mozilla/5.0 (Windows NT 6.1) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/41.0.2228.0 Safari/537.36',
            'Accept-Language': 'en-US, en;q=0.5'}

BASE_URL = 'https://www.amazon.com/s?k='
def encode_url(name):
    return BASE_URL + name.replace(' ', '+')

identifier = 'data-component-type="s-search-result"'

products = {}
with open('instacart-market-basket-analysis/products.csv', 'r') as f:
    reader = csv.reader(f)
    keys = next(reader)
    i = 0
    for row in reader:
        if i > 99:
            break
        i += 1
        time.sleep(1)
        request = requests.get(encode_url(row[1]), headers=(HEADERS))
        stripped = re.sub(r'<script>[.\n]*</script', '', request.content.decode('UTF-8'))
        soup = BeautifulSoup(stripped, features='lxml')
        results = soup.find_all('div', {'data-component-type': 's-search-result'})
        for result in results:
            # Unsponsored result
            if not result.find('span', {'class': 'aok-inline-block s-sponsored-label-info-icon'}):
                try:
                    dollar = result.find('span', {'class': 'a-price-whole'}).contents[0]
                    cents = result.find('span', {'class': 'a-price-fraction'}).contents[0]
                    cost = f'{dollar}.{cents}'
                    products[row[0]] = cost
                    break
                except AttributeError:
                    pass

with open('tmp.pickle', 'wb') as f:
    pickle.dump(products, f)

with open('prices.csv', 'w') as f:
    min_id = int(min(products.keys(), key=lambda x: int(x)))
    max_id = int(max(products.keys(), key=lambda x: int(x)))
    writer = csv.writer(f)
    for i in range(min_id, max_id+1):
        writer.writerow(products[str(i)])
