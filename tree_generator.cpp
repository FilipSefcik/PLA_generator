#include <algorithm>
#include <iostream>
#include <memory>
#include <numeric>
#include <queue>
#include <random>
#include <sstream>
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
  TreeGenerator(int md, int min_s, int max_s, int min_slots_, int max_slots_)
      : max_depth(md), min_sons(min_s), max_sons(max_s), min_slots(min_slots_),
        max_slots(max_slots_), rng(std::random_device{}()) {}

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
      int sons = random_range(min_sons, max_sons);
      sons = std::min(sons, node->slot_count);

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

  static std::string export_level_structure(const Node *root) {

    if (!root)
      return "";

    std::ostringstream nodes_info;
    std::ostringstream connections;

    std::queue<const Node *> q;
    q.push(root);

    while (!q.empty()) {

      const Node *current = q.front();
      q.pop();

      // ---- STRING 1 ----
      nodes_info << "N" << current->id << " " << current->slot_count << " 0\n";

      // ---- STRING 2 ----
      connections << "N" << current->id << " ";

      for (const auto &slot : current->slots) {

        if (slot) {
          connections << "N" << slot->id;
          q.push(slot.get());
        } else {
          connections << "V";
        }
      }

      connections << "\n";
    }

    std::ostringstream result;
    result << nodes_info.str() << "\n" << connections.str();

    return result.str();
  }
};

int main(int argc, char *argv[]) {

  if (argc != 6) {
    std::cout << "Usage:\n"
              << argv[0]
              << " <max_depth> <min_sons> <max_sons> <min_slots> <max_slots>\n";
    return 1;
  }

  int max_depth = std::stoi(argv[1]);
  int min_sons = std::stoi(argv[2]);
  int max_sons = std::stoi(argv[3]);
  int min_slots = std::stoi(argv[4]);
  int max_slots = std::stoi(argv[5]);

  if (min_sons < 0 || max_sons < 0 || min_slots <= 0 || max_slots <= 0 ||
      min_sons > max_sons || min_slots > max_slots) {
    std::cout << "Invalid parameters.\n";
    return 1;
  }

  TreeGenerator generator(max_depth, min_sons, max_sons, min_slots, max_slots);

  auto root = generator.generate_all_random();

  // ---- EXPORT STRUCTURE ----
  std::string output = TreeGenerator::export_level_structure(root.get());

  std::cout << output;

  //   std::cout << "\n--- PREORDER ---\n";
  //   TreeGenerator::preorder(root.get());

  //   std::cout << "\n--- POSTORDER ---\n";
  //   TreeGenerator::postorder(root.get());

  //   std::cout << "\n--- LEVEL ORDER ---\n";
  //   TreeGenerator::level_order(root.get());

  return 0;
}