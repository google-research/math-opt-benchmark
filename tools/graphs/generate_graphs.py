import graph_pb2
import random
import itertools
import os
import numpy as np

random.seed(12345)

SAVE_DIR = 'graph_protos/'
os.makedirs(SAVE_DIR, exist_ok=True)

MAX_DIST = 1

LOW_VERTICES = 25
HIGH_VERTICES = 75

for _ in range(100):
    print(_, end='\r')
    num_vertices = random.randint(LOW_VERTICES, HIGH_VERTICES)
    edge_cutoff = 2*np.log(num_vertices) / num_vertices
    vertices = []
    for i in range(num_vertices):
        x = random.random()
        y = random.random()
        vertices.append((x, y))
    connected = False
    attempt = -1
    while not connected:
        attempt += 1
        edges = []
        for edge in itertools.combinations(range(num_vertices), 2):
            if random.random() > edge_cutoff:
                edges.append(edge)
        incidence = np.zeros((len(edges), num_vertices), dtype='int')
        for i in range(len(edges)):
            v1, v2 = edges[i]
            incidence[i][v1] = 1
            incidence[i][v2] = -1
        laplacian = np.matmul(np.transpose(incidence), incidence)
        eigenvalues = sorted(np.linalg.eigvals(laplacian))
        connected = eigenvalues[1] > 0.00001
        # connected = np.abs(eigenvalues[0] - eigenvalues[1]) > 0.0001
        # Could also do adjacency to n-1 and check for 1
        print(f"not connected :( {attempt}", end='\r')

    graph = [[0 for j in range(num_vertices)] for i in range(num_vertices)]
    for v1, v2 in edges:
        v1x, v1y = vertices[v1]
        v2x, v2y = vertices[v2]
        graph[v1][v2] = graph[v2][v1] = ((v1x - v2x)**2 + (v1y - v2y)**2)**0.5

    proto_graph = graph_pb2.AdjacencyMatrix()
    for vertex_list in graph:
        adj = graph_pb2.AdjacencyList()
        adj.indices.extend(vertex_list)
        proto_graph.vertices.append(adj)
    with open(os.path.join(SAVE_DIR, f'{_}.pb'), 'wb') as f:
        f.write(proto_graph.SerializeToString())

