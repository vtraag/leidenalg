import igraph as ig
import louvain

G = ig.Graph.Erdos_Renyi(100, 0.1);
louvain.find_partition(G, "Modularity");
louvain.find_partition(G, "RBConfiguration");
louvain.find_partition(G, "Surprise");
louvain.find_partition(G, "Significance");

G.es['weight'] = 1.0;
louvain.find_partition(G, "Modularity", weight='weight');
louvain.find_partition(G, "RBConfiguration", weight='weight');
louvain.find_partition(G, "Surprise", weight='weight');
