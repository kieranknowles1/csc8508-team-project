#pragma once

#include <LinearMath/btVector3.h>
#include <LinearMath/btQuaternion.h>
#include <shared_mutex>
#include <unordered_map>
#include <variant>
#include <vector>

namespace WorldState {
    using StateValue = std::variant<btVector3, btQuaternion>;

    enum class StateType {
        LinearVelocity,
        AngularVelocity,
        Position,
        Rotation,
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
         * @see ReleaseReadLock();
         */
        inline void AcquireReadLock() { m_stateLock.lock_shared(); }

        /**
         * @brief Release previously acquired read lock.
         * @see AcquireReadLock
         */
        inline void ReleaseReadLock() { m_stateLock.unlock_shared(); }

        /**
         * @brief Update a state.
         * This function blocks reading until it has finished.
         */
        void UpdateState(const std::pair<StateType, StateValue>& stateUpdate) {
            std::unique_lock lock(m_stateLock);
            m_states[stateUpdate.first] = stateUpdate.second;
        }

        /**
         * @brief Write multiple state updates at once.
         * This function blocks reading until it has finished.
         */
        void UpdateStates(std::vector<std::pair<StateType, StateValue>>& stateUpdates) {
            std::unique_lock lock(m_stateLock);
            for (const std::pair<StateType, StateValue>& stateUpdate:stateUpdates) {
                m_states[stateUpdate.first] = stateUpdate.second;
            }
        }

        StateValue ReadState(StateType type) {
            std::shared_lock lock(m_stateLock);
            return m_states[type];
        }

        StateValue UnsafeReadState(StateType type) {
            return m_states[type];
        }

        inline int Size() const { return m_states.size(); }

        void Clear() {
            std::unique_lock lock(m_stateLock);
            m_states.clear();
        }

    private:
        std::shared_mutex m_stateLock;
        std::unordered_map<StateType, StateValue> m_states;
    };
}
