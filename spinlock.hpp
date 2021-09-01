#pragma once

#include <atomic>
#include <thread>
#include <concepts>

struct spinlock {
    std::atomic<bool> lock_ = {0};

    template <std::predicate P>
    void lock(P pred) noexcept {
        for (;;) {
        // Optimistically assume the lock is free on the first try
        if (pred() && !lock_.exchange(true, std::memory_order_acquire)) {
            if (pred()) {
                return;
            }
            else {
                unlock();
                // continue spinning
            }
        }
        // Wait for lock to be released without generating cache misses
        while (lock_.load(std::memory_order_relaxed)) {
            // Issue X86 PAUSE or ARM YIELD instruction to reduce contention between
            // hyper-threads
            // __builtin_ia32_pause(); 
            std::this_thread::yield();
        }
        }
    }

    void lock() noexcept {
        for (;;) {
        // Optimistically assume the lock is free on the first try
        if (!lock_.exchange(true, std::memory_order_acquire)) {
            return;
        }
        // Wait for lock to be released without generating cache misses
        while (lock_.load(std::memory_order_relaxed)) {
            std::this_thread::yield();
        }
        }
    }


    bool try_lock() noexcept {
        // First do a relaxed load to check if lock is free in order to prevent
        // unnecessary cache misses if someone does while(!try_lock())
        return !lock_.load(std::memory_order_relaxed) &&
            !lock_.exchange(true, std::memory_order_acquire);
    }

    void unlock() noexcept {
        lock_.store(false, std::memory_order_release);
    }
};