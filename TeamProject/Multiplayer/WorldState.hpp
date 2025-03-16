#pragma once

#include <LinearMath/btVector3.h>
#include <unordered_map>
#include <variant>
#include <shared_mutex>

namespace WorldState {
    using StateValue = std::variant<btVector3>;


    enum class StateType {
        LinearVelocity,
        AngularVelocity,
        Position,
        UpVector
    };


    /**
     * @brief A thread safe class for storing different states.
     */
    class ObjectState {
    public:
        ObjectState() {}
        ~ObjectState() {}

        /**
         * @brief Lock this object for reading if you want to read multiple
         * states at once without allowing writes.
         * @return A shared lock that will exist till destructed.
         */
        std::shared_lock<std::shared_mutex> GetReadLock() {
            return std::shared_lock(m_stateLock);
        }

        /**
         * @brief Lock this object for writing if you want to write to multiple
         * states at once. This will block reading until the lock is
         * destructed.
         * @return A unique lock that will exist till destructed.
         */
        std::unique_lock<std::shared_mutex> GetWriteLock() {
            return std::unique_lock(m_stateLock);
        }

        /**
         * @brief Update a state.
         * This function blocks all reading until it has finished.
         */
        template <typename T>
        void UpdateState(StateType type, T value) {
            std::unique_lock lock(m_stateLock);
            m_states[type] = value;
        }

        template <typename T>
        T ReadState(StateType type) {
            std::shared_lock lock(m_stateLock);
            return m_states[type];
        }

    private:
        std::shared_mutex m_stateLock;
        std::unordered_map<StateType, StateValue> m_states;
    };
}
