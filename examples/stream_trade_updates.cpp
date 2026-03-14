#include <alpaca/alpaca.hpp>
#include <alpaca/client/tradeUpdateStream.hpp>

#include <csignal>
#include <print>

static volatile bool g_running = true;

int main() {
  std::signal(SIGINT, [](int) { g_running = false; });

  alpaca::Environment env;

  alpaca::TradeUpdateCallbacks cbs;

  cbs.onConnected = [] {
    std::println("Trade update stream connected. Waiting for events...");
  };

  cbs.onDisconnected = [] { std::println("Disconnected."); };

  cbs.onUpdate = [](alpaca::TradeUpdate u) {
    std::string event;
    switch (u.event) {
    case alpaca::TradeUpdateEvent::new_order:
      event = "new";
      break;
    case alpaca::TradeUpdateEvent::fill:
      event = "fill";
      break;
    case alpaca::TradeUpdateEvent::partial_fill:
      event = "partial_fill";
      break;
    case alpaca::TradeUpdateEvent::canceled:
      event = "canceled";
      break;
    case alpaca::TradeUpdateEvent::replaced:
      event = "replaced";
      break;
    case alpaca::TradeUpdateEvent::rejected:
      event = "rejected";
      break;
    case alpaca::TradeUpdateEvent::pending_new:
      event = "pending_new";
      break;
    default:
      event = "other";
      break;
    }
    auto sym = u.order.symbol.value_or("?");
    std::println("UPDATE {} | {} | order={}", event, sym, u.order.id);
  };

  cbs.onError = [](alpaca::APIError e) {
    std::println("ERROR  [{}] {}", static_cast<int>(e.code), e.message);
  };

  alpaca::TradeUpdateStream stream(env);
  stream.Connect(std::move(cbs));

  std::println("Press Ctrl-C to stop.");
  while (g_running) {
    // spin; callbacks fire from ixwebsocket's background thread
  }

  stream.Disconnect();
  std::println("Done.");
}
