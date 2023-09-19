#include <condition_variable>
#include <iostream>
#include <queue>
#include <shared_mutex>
#include <thread>

struct Node {
  int value;
  Node* next{nullptr};
  std::mutex* node_mutex{nullptr};
};

class FineGrainedQueue {
  Node* head{nullptr};
  std::mutex* queue_mutex{new std::mutex};


  std::mutex cout_mutex;


 public:
  void insertIntoMiddle(int value, int pos) {
    Node* previous{nullptr};      // previous node
    Node* next{nullptr};          // next node (after insert)
    Node* inserted = new Node();  // inserted node
    inserted->value = value;
    inserted->node_mutex = new std::mutex;

    // на время поиска нужной позиции блокируем всю очередь,
    // иначе нельзя гарантировать, что нужная позиция не изменится

    // начинаем с головы
    queue_mutex->lock();
    if (!head) {
      head = new Node(*inserted);
      queue_mutex->unlock();

#if _DEBUG
      cout_mutex.lock();
      std::cout << "inserted in position: " << 0
                << " value: " << inserted->value << std::endl;
      cout_mutex.unlock();
#endif

      return;
    };
    previous = head;
    next = previous;
    int i{0};
    // ищем нужную позицию
    while (i != pos) {
      if (next == nullptr) break;
      previous = next;
      next = next->next;
      ++i;
    }

    if (!next) {
      head->node_mutex->lock();
      if (previous != head) previous->node_mutex->lock();
      queue_mutex->unlock();

      previous->next = inserted;

      if (previous != head) previous->node_mutex->unlock();
      head->node_mutex->unlock();

#if _DEBUG
      cout_mutex.lock();
      std::cout << "inserted in position: " << i
                << " value: " << inserted->value << std::endl;
      cout_mutex.unlock();
#endif

      return;
    }

    head->node_mutex->lock();
    if (previous != head) previous->node_mutex->lock();
    if (next != previous)
      next->node_mutex->lock();  // блокируем следующий элемент, чтобы
                                 // гарантировать, что его не удалят
    queue_mutex->unlock();

    inserted->next = next;
    previous->next = inserted;
    if (next != previous) next->node_mutex->unlock();
    if (previous != head) previous->node_mutex->unlock();
    head->node_mutex->unlock();

#if _DEBUG
    cout_mutex.lock();
    std::cout << "inserted in position: " << i << " value: " << inserted->value
              << std::endl;
    cout_mutex.unlock();
#endif

    return;
  }
};

int main() {
  FineGrainedQueue fineGrainedQueue;

  std::vector<std::thread> insertIntoMiddle_thread;

  insertIntoMiddle_thread.emplace_back(std::thread(
      &FineGrainedQueue::insertIntoMiddle, std::ref(fineGrainedQueue), 1, 1));
  insertIntoMiddle_thread.emplace_back(std::thread(
      &FineGrainedQueue::insertIntoMiddle, std::ref(fineGrainedQueue), 2, 2));
  insertIntoMiddle_thread.emplace_back(std::thread(
      &FineGrainedQueue::insertIntoMiddle, std::ref(fineGrainedQueue), 3, 3));
  insertIntoMiddle_thread.emplace_back(std::thread(
      &FineGrainedQueue::insertIntoMiddle, std::ref(fineGrainedQueue), 4, 1));
  insertIntoMiddle_thread.emplace_back(std::thread(
      &FineGrainedQueue::insertIntoMiddle, std::ref(fineGrainedQueue), 5, 2));
  insertIntoMiddle_thread.emplace_back(std::thread(
      &FineGrainedQueue::insertIntoMiddle, std::ref(fineGrainedQueue), 6, 3));
  insertIntoMiddle_thread.emplace_back(std::thread(
      &FineGrainedQueue::insertIntoMiddle, std::ref(fineGrainedQueue), 7, 1));
  insertIntoMiddle_thread.emplace_back(std::thread(
      &FineGrainedQueue::insertIntoMiddle, std::ref(fineGrainedQueue), 8, 2));
  insertIntoMiddle_thread.emplace_back(std::thread(
      &FineGrainedQueue::insertIntoMiddle, std::ref(fineGrainedQueue), 9, 3));

  for (auto& thread : insertIntoMiddle_thread) {
    if (thread.joinable()) {
      thread.join();
    }
  }

  return 0;
}