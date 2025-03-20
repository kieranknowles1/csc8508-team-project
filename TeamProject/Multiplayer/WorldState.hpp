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
        UpVector,
        StartPos,
        EndPos,
        Normal
    };


    /**
     * @brief A thread safe class for storing different states.
     */
    class ObjectState {
    public:
        ObjectState() {}
        ~ObjectState() {}

        void Lock() {
            m_stateLock.lock();
        }

        void Lock_Shared() {
            m_stateLock.lock_shared();
        }

        void Unlock() {
            m_stateLock.unlock();
        }

        void Unlock_Shared() {
            m_stateLock.unlock_shared();
        }

        /**
         * @brief Update a state.
         * This function blocks reading until it has finished.
         */
        void UpdateState(const StateType type, const StateValue& value) {
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
         * @brief Remove all states and their values.
         */
        void Clear() {
            m_states.clear();
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
            if (m_locked) {
                m_locked = false;
                m_readingMutex->unlock_shared();
            }
        }

        ObjectState* GetState() const { return m_state; }

    private:
        bool m_locked = true;
        ObjectState* m_state;
        std::shared_mutex* m_readingMutex;
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
        StateBuffer() {}
        ~StateBuffer() {}

        /**
         * @brief Get the current ObjectState.
         * 
         * Calling this function multiple times on the same thread without the
         * previous object going out of scope or without calling Unlock() is
         * undefined behaviour and will likely result in a deadlock.
         */
        StateReader GetCurrentState() { 
            m_stateMutexes[current].lock_shared();
            return StateReader(&m_states[current], &m_stateMutexes[current]);
        }

        /**
         * @brief Get the ObjectState to read from.
         * 
         * Calling this function multiple times on the same thread without the
         * previous object going out of scope or without calling Unlock() is
         * undefined behaviour and will likely result in a deadlock.
         */
        StateReader GetReadState() {
            m_stateMutexes[read].lock_shared();
            return StateReader(&m_states[read], &m_stateMutexes[read]);
        }

        /**
         * @brief Get the ObjectState to write to.
         * 
         * Calling this function multiple times on the same thread without the
         * previous object going out of scope or without calling Unlock() is
         * undefined behaviour and will likely result in a deadlock.
         */
        StateReader GetWriteState() {
            m_stateMutexes[write].lock_shared();
            return StateReader(&m_states[write], &m_stateMutexes[write]);
        }

        /**
         * @brief Update the world states.
         * 
         * Current becomes the previous read object state.
         * Read becomes the previous write object state.
         * Write is cleared.
         */
        void UpdateBuffer() {
            LockAll();

            current = read;
            read = write;
            write = (write + 1) % m_states.size();
            m_states[write].Clear();

            UnlockAll();
        }

    private:
        void LockAll() {
            for (int i = 0; i < m_stateMutexes.size(); i++) {
                m_stateMutexes[i].lock();
            }
        }

        void UnlockAll() {
            for (int i = 0; i < m_stateMutexes.size(); i++) {
                m_stateMutexes[i].unlock();
            }
        }

        std::array<ObjectState, 3> m_states;
        std::array<std::shared_mutex, 3> m_stateMutexes;
        
        int current = 0;
        int read = 1;
        int write = 2;
    };
}
