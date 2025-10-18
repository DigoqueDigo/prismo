#ifndef ENGINE_UTILS_H
#define ENGINE_UTILS_H

#include <concepts>

template <typename EngineT, typename LoggerT, typename MetricT>
concept HasTemplatedReapLeftCompletions = requires(EngineT& e, LoggerT& logger) {
    { e.template reap_left_completions<LoggerT, MetricT>(logger) } -> std::same_as<void>;
};

template <typename EngineT, typename LoggerT, typename MetricT>
void maybe_reap_left_completions(EngineT& engine, LoggerT& logger) {
    if constexpr (HasTemplatedReapLeftCompletions<EngineT, LoggerT, MetricT>) {
        engine.template reap_left_completions<LoggerT, MetricT>(logger);
    }
}

#endif