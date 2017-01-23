import analysis_tools as at
import pandas as pd
import os
import numpy as np
import igraph as ig
from scipy import sparse

#%%
def benchmark_network(n, k, maxk, mu, prop_overlap_nodes=0, nb_overlap_clusters=0, t1=2, t2=1, minc=10, maxc=50):
  
  nb_overlap_nodes = prop_overlap_nodes*n;
  while True:
    str_exec = '~/bin/lfr_benchmark_binary'
    str_exec += ' -N {0} -k {1} -maxk {2} '.format(n, k, maxk);
    str_exec += ' -mu {0} -t1 {1} -t2 {2} '.format(mu, t1, t2);
    str_exec += ' -minc {0} -maxc {1} '.format(minc, maxc);
    str_exec += ' -on {0} -om {1}'.format(nb_overlap_nodes, nb_overlap_clusters);
    str_exec += ' > /dev/null'
    os.system(str_exec);
    if os.path.exists('network.dat'):
      break;
  edgelist_df = pd.read_csv('network.dat', 
      names=['source', 'target'], sep='\t');
  node_df = pd.read_csv('community.dat', names=['id',
    'planted_membership'], sep='\t', 
    converters={'planted_membership': str});
  node_df['planted_membership'] = node_df['planted_membership'].apply(
    lambda x: 
        map(lambda y: int(y) - 1, 
            x.strip().split(' ')
            )
    );
  G = at.graph.GraphFromPandas(edgelist_df, node_df);

  return G;
    
def overlapping_cliques(nc=5, r=2, overlap=1, circular=False):

  cl = np.ones([nc, nc]) - np.eye(nc);

  n = r*nc - r*overlap;
  if (not circular):
    n += overlap;

  A = np.zeros([n, n]);
  for c in range(r - 1):
    A[c*nc - c*overlap:(c+1)*nc - c*overlap,
      c*nc - c*overlap:(c+1)*nc - c*overlap] = cl;

  c = r - 1;    
  if not circular:
    A[c*nc - c*overlap:(c+1)*nc - c*overlap,
      c*nc - c*overlap:(c+1)*nc - c*overlap] = cl;
  else:
    A[c*nc - c*overlap:(c+1)*nc - c*overlap,
      c*nc - c*overlap:(c+1)*nc - c*overlap] = cl[0:-overlap,0:-overlap];
    A[0:overlap+1,c*nc - c*overlap:(c+1)*nc - c*overlap] = 1;
    A[c*nc - c*overlap:(c+1)*nc - c*overlap,0:overlap+1] = 1;

  G = ig.Graph.Adjacency(A.tolist(), mode=ig.ADJ_UNDIRECTED);
  #%%
  return G;

def block_model(n_c=10, r=5, k=5, mu=0):
  n = r*n_c;
  p_in = (1 - mu)*k/float(n_c - 1);
  p_out = mu*k/float(n - n_c);
  M = p_in*np.eye(r) + p_out*(np.ones(r) - np.eye(r));
  # Rescale to obtain average degree
  #G = ig.Graph.Preference(n, type_dist.tolist(), M.tolist(), 'community');
  G = ig.Graph.SBM(n, M.tolist(), block_sizes = [n_c]*r)
  G.vs['community'] = [int(v/n_c) for v in range(n)];
  return G;


def clusters_from_partition(part):
    membership_matrix_edges = np.array([(i, c) 
          for i, c in enumerate(part.membership)]);
    data = np.repeat(1, len(membership_matrix_edges));
    S = sparse.coo_matrix( (data, membership_matrix_edges.T), 
                            shape=(len(part.membership), len(part)) );
    S = S.tocsc();
    return [list(S[:,col].nonzero()[0]) for col in range(S.shape[1])];
