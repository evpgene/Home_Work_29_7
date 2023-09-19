template <class T>
class CVQueue
{
  std::queue<T> internal;
  std::shared_mutex mutex;
  std::shared_mutex* queue_mutex;

//   std::unique_mutex queue_mutex;
//   std::unique_mutex node_mutex;
  std::condition_variable_any data_cond; // объект условной переменной для синхронизации методов
  Node* head;

public:
  void push(const T& value)
  {
    std::unique_lock<std::shared_mutex> guard(mutex);
    internal.push(value);
    data_cond.notify_one(); // пробуждаем один ожидающий поток c вызовом pop(), если таковые имеются
  }

  void pop()
  {
    std::unique_lock<std::shared_mutex> lock(mutex);

    // дожидаемся, пока в очередь добавят элемент
    data_cond.wait(lock, [this] { return !internal.empty(); }); 
    
    internal.pop();
  }

  auto size()
  {
    std::shared_lock<std::shared_mutex>(mutex);
    auto result = internal.size();
    return result;
  }

  void remove(int value)
  {
    Node *prev, *cur;
    queue_mutex->lock();

    // здесь должна быть обработка случая пустого списка

    prev = this->head;
    cur = this->head->next;

    prev->node_mutex->lock();
    // здесь должна быть обработка случая удаления первого элемента
    queue_mutex->unlock();

    if (cur) // проверили и только потом залочили
      cur->node_mutex->lock();
    while (cur)
    {
      if (cur->value == value)
      {
        prev->next = cur->next;
        prev->node_mutex->unlock();
        cur->node_mutex->unlock();
        delete cur;
        return;
      }
      Node* old_prev = prev;
      prev = cur;
      cur = cur->next;
      old_prev->node_mutex->unlock();
      if (cur) // проверили и только потом залочили
        cur->node_mutex->lock();
    }
    prev->node_mutex->unlock();
  }
};