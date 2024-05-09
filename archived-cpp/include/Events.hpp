#ifndef EVENTHANDLER_H
#define EVENTHANDLER_H

template <typename EventT>
class EventHandler
{
  public:
  EventHandler() = default;
  virtual ~EventHandler() = default;

  // Returns if it handled the event successfully/ate the event, so it doesn't get passed down the chain.
  virtual bool handle(EventT& e) = 0;
};

template <typename EventT>
class EventReporter
{
  public:
  EventHandler<EventT>* listener = nullptr;

  // Returns if any listener handled the event at all, may return false if no listener is set.
  bool report(EventT& e)
  {
    if (listener == nullptr)
    {
      return false;
    }
    return listener->handle(e);
  }
};

#endif // EVENTHANDLER_H
