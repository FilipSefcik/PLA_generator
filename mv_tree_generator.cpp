#include <algorithm>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <memory>
#include <numeric>
#include <queue>
#include <random>
#include <sstream>
#include <string>
#include <vector>

class Node {
public:
  int id;
  int depth;
  int slot_count;
  std::vector<std::unique_ptr<Node>> slots;

  Node(int id_, int depth_, int slot_count_)
      : id(id_), depth(depth_), slot_count(slot_count_), slots(slot_count_) {}
};

class TreeGenerator {
private:
  int max_depth;
  int min_sons;
  int max_sons;
  int min_slots;
  int max_slots;

  int global_id = 0;
  std::mt19937 rng;

  int random_range(int min, int max) {
    std::uniform_int_distribution<int> dist(min, max);
    return dist(rng);
  }

public:
  TreeGenerator(int md, int min_s, int max_s, int min_slots_, int max_slots_,
                unsigned int seed = std::random_device{}())
      : max_depth(md), min_sons(min_s), max_sons(max_s), min_slots(min_slots_),
        max_slots(max_slots_), rng(seed) {}

  std::unique_ptr<Node> generate_with_backbone() {

    auto root = std::make_unique<Node>(global_id++, 0,
                                       random_range(min_slots, max_slots));

    std::vector<Node *> frontier;
    frontier.push_back(root.get());

    // ---- Step 1: Guarantee deep path ----

    Node *current = root.get();

    for (int d = 0; d < max_depth; ++d) {

      int new_slots = random_range(min_slots, max_slots);

      std::vector<int> positions(current->slot_count);
      std::iota(positions.begin(), positions.end(), 0);
      std::shuffle(positions.begin(), positions.end(), rng);

      int pos = positions[0];

      current->slots[pos] =
          std::make_unique<Node>(global_id++, d + 1, new_slots);

      current = current->slots[pos].get();
      frontier.push_back(current);
    }

    // ---- Step 2: Random expansion ----

    frontier.clear();
    frontier.push_back(root.get());

    for (int depth = 0; depth < max_depth; ++depth) {

      std::vector<Node *> next_frontier;

      for (Node *node : frontier) {

        int sons = random_range(min_sons, max_sons);
        sons = std::min(sons, node->slot_count);

        std::vector<int> empty_positions;
        for (int i = 0; i < node->slot_count; ++i)
          if (!node->slots[i])
            empty_positions.push_back(i);

        if (empty_positions.empty())
          continue;

        std::shuffle(empty_positions.begin(), empty_positions.end(), rng);

        int create_count = std::min(sons, (int)empty_positions.size());

        for (int i = 0; i < create_count; ++i) {

          int pos = empty_positions[i];

          int new_slots = random_range(min_slots, max_slots);

          node->slots[pos] =
              std::make_unique<Node>(global_id++, depth + 1, new_slots);

          next_frontier.push_back(node->slots[pos].get());
        }
      }

      frontier = std::move(next_frontier);

      if (frontier.empty())
        break;
    }

    return root;
  }

  std::unique_ptr<Node> generate_all_random() {
    // Root gets random slot count too
    int root_slots = random_range(min_slots, max_slots);
    auto root = std::make_unique<Node>(global_id++, 0, root_slots);

    // Recursive lambda for pure random growth
    auto grow = [&](auto &&self, Node *node) -> void {
      if (!node)
        return;

      // Stop if max depth reached: leaf
      if (node->depth >= max_depth)
        return;

      // Choose how many sons this node will have
      int sons = 0;
      do {
        sons = random_range(min_sons, max_sons);
        sons = std::min(sons, node->slot_count);
      } while (sons <= 0 && node->depth == 0);

      if (sons <= 0)
        return; // leaf due to randomness

      // Choose random unique positions (among this node's slots)
      std::vector<int> positions(node->slot_count);
      std::iota(positions.begin(), positions.end(), 0);
      std::shuffle(positions.begin(), positions.end(), rng);

      for (int i = 0; i < sons; ++i) {
        int pos = positions[i];

        int child_slots = random_range(min_slots, max_slots);
        node->slots[pos] =
            std::make_unique<Node>(global_id++, node->depth + 1, child_slots);

        self(self, node->slots[pos].get());
      }
    };

    grow(grow, root.get());
    return root;
  }

  // -----------------------
  // Traversals
  // -----------------------

  static void preorder(const Node *node) {
    if (!node)
      return;

    std::cout << "Node " << node->id << " | depth: " << node->depth
              << " | children: ";

    int count = 0;
    for (const auto &child : node->slots)
      if (child)
        count++;

    std::cout << count << "\n";

    for (const auto &child : node->slots)
      if (child)
        preorder(child.get());
  }

  static void postorder(const Node *node) {
    if (!node)
      return;

    for (const auto &child : node->slots)
      if (child)
        postorder(child.get());

    std::cout << "Node " << node->id << " | depth: " << node->depth << "\n";
  }

  static void level_order(const Node *root) {
    if (!root)
      return;

    std::queue<const Node *> q;
    q.push(root);

    while (!q.empty()) {
      const Node *current = q.front();
      q.pop();

      int count = 0;
      for (const auto &child : current->slots)
        if (child)
          count++;

      std::cout << "Node " << current->id << " | depth: " << current->depth
                << " | children: " << count << "\n";

      for (const auto &child : current->slots)
        if (child)
          q.push(child.get());
    }
  }

  // -----------------------------
  // LEVEL ORDER EXPORT FUNCTION
  // -----------------------------
  static bool generate_pla_file(int row_size, int num_states,
                                const std::string &output_filename) {
    if (row_size <= 0 || num_states <= 0)
      return false;

    const int num_inputs = row_size;
    const int total_vars = num_inputs + 1;

    // "number of rows will always be number of variables times number of
    // variables"
    const int rows = total_vars * total_vars;

    std::ofstream f(output_filename);
    if (!f.is_open())
      return false;

    std::mt19937 rng(std::random_device{}());
    std::uniform_int_distribution<int> pos_dist(0, num_states - 1);

    // ---- STRING 1: .mv header ----
    f << ".mv " << total_vars << " 0";
    for (int i = 0; i < total_vars; ++i) {
      f << " " << num_states; // Homogeneous state count
    }
    f << "\n";

    // ---- STRING 2: .p header ----
    f << ".p " << rows << "\n";

    // ---- STRING 3: Rows generation ----
    for (int r = 0; r < rows; r++) {
      for (int v = 0; v < total_vars; v++) {
        f << "|";

        int active_pos = pos_dist(rng);

        for (int s = 0; s < num_states; s++) {
          f << (s == active_pos ? '1' : '0');
        }
      }
      f << "\n";
    }

    f << ".e\n";
    f.close();

    return true;
  }

  static std::string
  export_level_structure_as_config(const Node *root, const std::string &pla_dir,
                                   int num_states) { // <-- Added parameter here
    if (!root)
      return "";

    std::filesystem::create_directories(pla_dir);

    std::ostringstream modules_info;
    std::ostringstream connections;

    std::queue<const Node *> q;
    q.push(root);

    while (!q.empty()) {
      const Node *current = q.front();
      q.pop();

      std::string pla_path =
          pla_dir + "/M" + std::to_string(current->id) + ".pla";

      // <-- Pass num_states into generate_pla_file
      if (!generate_pla_file(current->slot_count, num_states, pla_path)) {
        std::cerr << "Failed to generate PLA: " << pla_path << "\n";
      }

      modules_info << "M" << current->id << " " << pla_path << " 0\n";
      connections << "M" << current->id << " ";

      for (const auto &slot : current->slots) {
        if (slot) {
          connections << "M" << slot->id;
          q.push(slot.get());
        } else {
          connections << "V";
        }
      }
      connections << "\n";
    }

    std::ostringstream result;
    result << modules_info.str() << "\n" << connections.str();
    return result.str();
  }
};

int main(int argc, char *argv[]) {
  // Update argument count to 9
  if (argc != 9) {
    std::cout << "Usage:\n"
              << argv[0]
              << " <max_depth> <min_sons> <max_sons> <min_slots> <max_slots> "
                 " <num_states> <seed> <pla_dir>\n";
    return 1;
  }

  int max_depth = std::stoi(argv[1]);
  int min_sons = std::stoi(argv[2]);
  int max_sons = std::stoi(argv[3]);
  int min_slots = std::stoi(argv[4]);
  int max_slots = std::stoi(argv[5]);
  int num_states = std::stoi(argv[6]); // <-- Parse the new parameter
  int seed = std::stoi(argv[7]);
  std::string pla_dir = argv[8];

  if (min_sons < 0 || max_sons < 0 || min_slots <= 0 || max_slots <= 0 ||
      min_sons > max_sons || min_slots > max_slots || max_depth <= 0 ||
      seed < 0 || pla_dir.empty() || num_states <= 0) { // <-- Add validation
    std::cout << "Invalid parameters.\n";
    return 1;
  }

  TreeGenerator generator(max_depth, min_sons, max_sons, min_slots, max_slots,
                          seed);
  auto root = generator.generate_all_random();

  // Pass num_states into the export function
  std::string config_output = TreeGenerator::export_level_structure_as_config(
      root.get(), pla_dir, num_states);

  // Create the config path as string concatenation
  std::string config_path = pla_dir + "/system.conf";

  // Write config file
  std::ofstream out_file(config_path);
  if (!out_file) {
    std::cerr << "Failed to write config file: " << config_path << "\n";
    return 1;
  }

  out_file << config_output;
  out_file.close();

  std::cout << "Configuration saved to: " << config_path << "\n";

  // Additional outputs
  std::cout << "Seed: " << seed << "\n";

  // Count the number of nodes and the number of variables (slots)
  int node_count = 0;
  int variable_count = 0;

  // Traverse the tree to count nodes and slots
  std::function<void(const Node *)> count_nodes_and_slots =
      [&](const Node *node) {
        if (!node)
          return;
        node_count++; // Count the current node
        variable_count +=
            node->slot_count; // Add the number of slots (variables)
        for (const auto &child : node->slots) {
          count_nodes_and_slots(child.get());
        }
      };

  count_nodes_and_slots(root.get());

  std::cout << "Number of modules: " << node_count << "\n";
  std::cout << "Number of variables: " << variable_count << "\n";
  std::cout << "Tree depth: " << max_depth << "\n";

  return 0;
}