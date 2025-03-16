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
         * @brief Update a state.
         * This function blocks all reading until it has finished.
         */
        void UpdateState(StateType type, StateValue value) {
            std::unique_lock lock(m_stateLock);
            m_states[type] = value;
        }

        /**
         * @brief Write multiple states updates at once.
         * Assumes state and value parameters have the same number of items.
         */
        void UpdateStates(std::vector<StateType>& states, std::vector<StateValue>& values) {
            std::unique_lock lock(m_stateLock);
            for (int i = 0; i < states.size(); i++) {
                m_states[states[i]] = values[i];
            }
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
