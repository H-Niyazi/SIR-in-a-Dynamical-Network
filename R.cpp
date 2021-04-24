#include <iostream>
#include <vector>
#include <unordered_map>
#include <unordered_set>
#include <cstdlib>
#include <ctime>
using namespace std;

struct node
{
  int id;            // id of the node
  int cluster;       // which cluster the node belongs to
  bool color;        // 0 for blue, 1 for red
  char inf = 's';    // infection status: 's', 'i', or 'r'
};

double same_cluster_prob(vector<node>& nodes)
{
  double freq = 0;

  for(int i = 0; i < nodes.size(); i++)
    for(int j = i+1; j < nodes.size(); j++)
      if(nodes[i].cluster == nodes[j].cluster)
        freq++;

  int N = nodes.size();
  return freq/(N*(N-1)/2.);
}


int main(int argc, char** argv)
{
  int N             = atoi(argv[1]);  // total number of particles
  double p_blue     = atof(argv[2]);  // portion of blue nodes
  double p_fragm_r  = atof(argv[3]);  // red  fragmentation probability
  double p_fragm_b  = atof(argv[4]);  // blue fragmentation probability
  double p_merge_rr = atof(argv[5]);  // red  and red  merging
  double p_merge_rb = atof(argv[6]);  // red  and blue merging
  double p_merge_bb = atof(argv[7]);  // blue and blue merging
  double p_inf      = atof(argv[8]);  // probability of infection
  double p_rec      = atof(argv[9]);  // probability of recovery
  int therm         = atoi(argv[10]); // thermalization steps before infction
  int steps         = atoi(argv[11]); // total number of steps

  vector<node> nodes(N);  // this is an array which stores all the nodes
  unordered_map< int, unordered_set<int> > clusters;
  unordered_map< int, unordered_set<int> > infected_clusters;
  unordered_set<int> infected_nodes;
  unordered_set<int> recovered_nodes;

  srand((unsigned)time(0));

  for(int i = 0; i < N; i++)
  {
    nodes[i].id = i;  // node number i

    int cl = rand()%N;  // a random number for the cluster that
                                  // node i belongs to
    nodes[i].cluster = cl;

    if(i < N*p_blue)
      nodes[i].color = 0;
    else
      nodes[i].color = 1;

    clusters[cl].insert(i);  // we save the node id to the corresponding
                             // cluster
  }

  int last_cluster = N;


  // initial thermalization
  for(int t = 0; t < therm; t++)
  {
    // this part is for the collapsing
    int node_id = rand()%N;  // we choose a random node
    int node_cluster = nodes[node_id].cluster;

    double p_collapse;
    if(nodes[node_id].color == 0)
      p_collapse = p_fragm_b;
    else
      p_collapse = p_fragm_r;

    double r = (double)rand()/RAND_MAX;  // a random number between 0 and 1

    if(r < p_collapse)
    {
      for(int b:clusters[node_cluster])
      {
        nodes[b].cluster = last_cluster;
        clusters[last_cluster].insert(nodes[b].id);
        last_cluster++;
      }

      clusters.erase(node_cluster);
    }

    // this part is for the merging
    int node_one_id = rand()%N;  // we choose a       random node
    int node_two_id = rand()%N;  // we choose another random node

    bool node_one_color = nodes[node_one_id].color;
    bool node_two_color = nodes[node_two_id].color;

    int node_one_cluster = nodes[node_one_id].cluster;
    int node_two_cluster = nodes[node_two_id].cluster;

    double p_merge;
    if(node_one_color == 0 && node_two_color == 0)
      p_merge = p_merge_bb;
    else if(node_one_color == 1 && node_two_color == 1)
      p_merge = p_merge_rr;
    else
      p_merge = p_merge_rb;

    r = (double)rand()/RAND_MAX;

    if(r < p_merge)
    {
      if(node_two_cluster != node_one_cluster)
      {
        for(int b:clusters[node_two_cluster])
        {
          nodes[b].cluster = node_one_cluster;
          clusters[node_one_cluster].insert(nodes[b].id);
        }

        clusters.erase(node_two_cluster);
      }
    }
  }


  // we find the largest cluster
  int max_cluster_size = 0;
  int largest_cluster;

  for(auto c:clusters)
    if(c.second.size() > max_cluster_size)
    {
      max_cluster_size = c.second.size();
      largest_cluster  = c.first;
    }

  // we choose some nodes random node from the largest cluster to be infected
  unordered_set<int>::iterator iter = clusters[largest_cluster].begin();
  int infected = *iter;
  nodes[infected].inf = 'i';
  infected_nodes.insert(infected);
  infected_clusters[largest_cluster].insert(infected);

  while(++iter != clusters[largest_cluster].end())
    if( (double)rand() / RAND_MAX < 0.5 )
    {
      infected = *iter;
      nodes[infected].inf = 'i';
      infected_nodes.insert(infected);
      infected_clusters[largest_cluster].insert(infected);
    }

  cout << "\nFirst infected cluster was a cluster of size: "
       << max_cluster_size << endl;

  for(int t = 1; t <= steps; t++)
  {
    double r;  // used for generating a random number between 0 and 1 later

    unordered_set<int> prev_infected_nodes = infected_nodes;
    unordered_map< int, unordered_set<int> > prev_infected_clusters =
                                                  infected_clusters;

    // all the susceptible nodes in the same cluster as an infected node have a
    // possibility for infection
    for(auto c:prev_infected_clusters)
      for(int n:clusters[c.first])
      {
        r = (double)rand()/RAND_MAX;
        if(nodes[n].inf == 's' && r < p_inf)
        {
          nodes[n].inf = 'i';
          infected_nodes.insert(n);
          infected_clusters[c.first].insert(n);
        }
      }

    // all the previously infected nodes have a chance of recovery
    for(int n:prev_infected_nodes)
    {
      int c = nodes[n].cluster;
      r = (double)rand()/RAND_MAX;
      if(r < p_rec)
      {
        if(infected_clusters[c].size() == 1)
          infected_clusters.erase(c);
        else
          infected_clusters[c].erase(n);

        nodes[n].inf = 'r';
        infected_nodes.erase(n);
        recovered_nodes.insert(n);
      }
    }

    if(infected_nodes.size() == 0)
    {
      cout << "Disease has vanished from the populations at time t="
           << t << " !\n" << recovered_nodes.size()*100./N << "\% of the total"
           << " population contracted the disease and recovered!\n\n";

      break;
    }


    // this part is for the collapsing
    int node_id = rand()%N;  // we choose a random node
    int node_cluster = nodes[node_id].cluster;

    double p_collapse;
    if(nodes[node_id].color == 0)
      p_collapse = p_fragm_b;
    else
      p_collapse = p_fragm_r;

    r = (double)rand()/RAND_MAX;

    if(r < p_collapse)
    {
      if(infected_clusters.find(node_cluster) != infected_clusters.end())
        infected_clusters.erase(node_cluster);

      for(int n:clusters[node_cluster])
      {
        nodes[n].cluster = last_cluster;
        clusters[last_cluster].insert(n);
        if(nodes[n].inf == 'i')
          infected_clusters[last_cluster].insert(n);
        last_cluster++;
      }

      clusters.erase(node_cluster);
    }


    // this part is for the merging
    int node_one_id = rand()%N;  // we choose a       random node
    int node_two_id = rand()%N;  // we choose another random node

    bool node_one_color = nodes[node_one_id].color;
    bool node_two_color = nodes[node_two_id].color;

    int node_one_cluster = nodes[node_one_id].cluster;
    int node_two_cluster = nodes[node_two_id].cluster;

    double p_merge;
    if(node_one_color == 0 && node_two_color == 0)
      p_merge = p_merge_bb;
    else if(node_one_color == 1 && node_two_color == 1)
      p_merge = p_merge_rr;
    else
      p_merge = p_merge_rb;

    r = (double)rand()/RAND_MAX;

    if(r < p_merge && node_one_cluster != node_two_cluster)
    {
      bool cluster_one_infected =
           infected_clusters.find(node_one_cluster) != infected_clusters.end();
      bool cluster_two_infected =
           infected_clusters.find(node_two_cluster) != infected_clusters.end();

      if(!cluster_one_infected && !cluster_two_infected)
      {
        for(int b:clusters[node_two_cluster])
        {
          nodes[b].cluster = node_one_cluster;
          clusters[node_one_cluster].insert(b);
        }

        clusters.erase(node_two_cluster);
      }

      else if(cluster_one_infected && cluster_two_infected)
      {
        for(int b:clusters[node_two_cluster])
        {
          nodes[b].cluster = node_one_cluster;
          clusters[node_one_cluster].insert(b);
          infected_clusters[node_one_cluster].insert(b);
        }

        clusters.erase(node_two_cluster);
        infected_clusters.erase(node_two_cluster);
      }

      else if(cluster_one_infected && !cluster_two_infected)
      {
        for(int b:clusters[node_two_cluster])
        {
          nodes[b].cluster = node_one_cluster;
          clusters[node_one_cluster].insert(b);
          infected_clusters[node_one_cluster].insert(b);
        }

        clusters.erase(node_two_cluster);
      }

      else
      {
        for(int b:clusters[node_one_cluster])
        {
          nodes[b].cluster = node_two_cluster;
          clusters[node_two_cluster].insert(b);
          infected_clusters[node_two_cluster].insert(b);
        }

        clusters.erase(node_one_cluster);
      }
    }

  }

  cout << "The ratio of all the infected people to the initially infected "
       << "cluster:\nR= " << (double)recovered_nodes.size()/max_cluster_size
       << endl << endl;


  return 0;
}
