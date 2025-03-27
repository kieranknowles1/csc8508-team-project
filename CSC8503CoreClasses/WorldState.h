#pragma once

#include <array>
#include <LinearMath/btVector3.h>
#include <LinearMath/btQuaternion.h>
#include <shared_mutex>
#include <unordered_map>
#include <variant>
#include <vector>
#include <optional>
#include <mutex>

namespace WorldState {
    using StateValue = std::variant<btVector3, btQuaternion, float, int>;

    enum class StateType {
        LinearVelocity,
        AngularVelocity,
        Position,
        Rotation,
        UpVector,
        StartPos,
        EndPos,
        Normal,
        DamageDealt,
        Health,
        ObjectID,
        Score,
        ScoreIncrease,
        Animation
    };


    /**
     * @brief A thread safe class for storing different states.
     */
    class ObjectState {
    public:
        ObjectState() {}
        ~ObjectState() {}

        std::unique_lock<std::shared_mutex> Lock() {
            return std::move(std::unique_lock(m_stateLock));
        }

        std::shared_lock<std::shared_mutex> Lock_Shared() {
            return std::move(std::shared_lock(m_stateLock));
        }

        /**
         * @brief Update a state.
         * This function blocks reading until it has finished.
         */
        void UpdateState(const StateType type, const StateValue value) {
            m_states[type] = value;
        }

        /**
         * @brief Read a state. If the state does not exist returns false.
         */
        bool ReadState(StateType type, StateValue* value) {
            if (m_states.contains(type)) {
                *value = m_states[type];
                return true;
            }
            return false;
        }

        /**
         * If they key exists, it will remove the value from the object state.
         */
        void RemoveState(StateType type) {
            m_states.erase(type);
        }

        /**
         * @brief Remove all states and their values.
         */
        void Clear() {
            m_states.clear();
            //m_states = std::unordered_map<StateType, StateValue>();
        }

        /**
         * @brief The number of states stored in this object.
         */
        inline int Size() const { return m_states.size(); }

    private:
        std::shared_mutex m_stateLock;
        std::unordered_map<StateType, StateValue> m_states;
    };


    /**
     * @brief An object returned by the state buffer for reading and writing to
     * a state buffer.
     *
     * The reading mutex does not handling reading and writing operations to
     * the state but handles locking / unlocking the state buffer update.
     */
    class StateReader {
    public:
        StateReader(ObjectState* state, std::shared_mutex* readingMutex) {
            m_state = state;
            m_readingMutex = readingMutex;
        }

        // Prevent copying as locking the same mutex multiple times on the same
        // Thread is undefined behaviour.
        StateReader(const StateReader& other) = delete;

        StateReader(StateReader&& other) noexcept {
            other.m_state = m_state;
            other.m_readingMutex = m_readingMutex;
            other.m_readingMutex = nullptr;
        }

        ~StateReader() {
            Unlock();
        }

        /**
         * @brief Unlock the reading mutex.
         *
         * This function invalidates this object and any attempt to use this
         * object after calling this function is undefined behaviour.
         */
        void Unlock() {
            if (m_locked && m_readingMutex != nullptr) {
                m_locked = false;
                m_readingMutex->unlock_shared();
            }
        }

        ObjectState* GetState() const { return m_state; }

    private:
        bool m_locked = true;
        ObjectState* m_state = nullptr;
        std::shared_mutex* m_readingMutex = nullptr;
    };


    /**
     * @brief State buffer contains 3 different buffers for reading, writing
     * and interpolating world states.
     *
     * It's main use it to interpolate world states between current and read
     * while still allowing writes to the write state so that state swapping is
     * smooth and without delay.
     */
    class StateBuffer {
    public:
        /**
         * @brief Get the current ObjectState.
         *
         * Calling this function multiple times on the same thread without
         * unlocking will most likely result in a deadlock.
         */
        std::pair<ObjectState*, std::shared_lock<std::shared_mutex>> GetCurrentState() {
            std::shared_lock lock(m_stateMutexes[current]);
            return std::make_pair(&m_states[current], std::move(lock));
        }

        /**
         * @brief Get the ObjectState to read from.
         *
         * Calling this function multiple times on the same thread without
         * unlocking will most likely result in a deadlock.
         */
        std::pair<ObjectState*, std::shared_lock<std::shared_mutex>> GetReadState() {
            std::shared_lock lock(m_stateMutexes[read]);
            return std::make_pair(&m_states[read], std::move(lock));
        }

        /**
         * @brief Get the ObjectState to write to.
         *
         * Calling this function multiple times on the same thread without
         * unlocking will most likely result in a deadlock.
         */
        std::pair<ObjectState*, std::shared_lock<std::shared_mutex>> GetWriteState() {
            std::shared_lock lock(m_stateMutexes[write]);
            return std::make_pair(&m_states[write], std::move(lock));
        }

        /**
         * @brief Update the world states.
         *
         * Current becomes the previous read object state.
         * Read becomes the previous write object state.
         * Write is cleared.
         */
        void UpdateBuffer() {
            std::unique_lock currentLock(m_stateMutexes[current]);
            std::unique_lock readLock(m_stateMutexes[read]);
            std::unique_lock writeLock(m_stateMutexes[write]);

            current = read;
            read = write;
            write = (write + 1) % m_states.size();
            m_states[write].Clear();
        }

    private:
        std::array<std::shared_mutex, 3> m_stateMutexes;
        std::array<ObjectState, 3> m_states;

        int current = 0;
        int read = 1;
        int write = 2;
    };
}
