//
// Created by Florian Damiot on 13/02/2023.
//

#include <functional>
#include <algorithm>
#include <cstdint>
#include <map>
#include <memory>
#include <any>
#include <vector>

    /**
     * @brief This class is used to register, unregister listeners
     * and trigger events.
     * @tparam EventType The type of the event.
     */
    class EventRegistry
    {
    public:
        /**
         * @brief Construct a new Event Registry object.
         */
        EventRegistry() = default;

        /**
         * @brief Register a handler for the event.
         * @param handler The handler to register.
         */
        template <typename EventType>
        void registerHandler(const uint32_t &packerHeaderId,
                             const std::shared_ptr<std::function<void(EventType e)>> &handler)
        {
            auto it = _mEventHandlers.find(packerHeaderId);

            if (it == _mEventHandlers.end())
            {
                std::vector<std::shared_ptr<std::function<void(EventType)>>> v;
                v.push_back(handler);
                _mEventHandlers[packerHeaderId] = v;
            }
            else
            {
                std::vector<std::shared_ptr<std::function<void(EventType)>>> v =
                        std::any_cast<std::vector<std::shared_ptr<std::function<void(EventType)>>>>(_mEventHandlers[packerHeaderId]);
                v.push_back(handler);
                _mEventHandlers[packerHeaderId] = v;
            }
        }

        /**
         * @brief Unregister a handler for the event.
         * @param handler The handler to register.
         * @deprecated Impossible to use the comparator == to compare std::function.
         */
        template <typename EventType>
        void unregisterHandler(const uint32_t &packerHeaderId,
                               const std::shared_ptr<std::function<void(EventType e)>> &handler)
        {
            auto it = _mEventHandlers.find(packerHeaderId);
            if (it == _mEventHandlers.end())
            {
                return;
            }
            std::vector<std::shared_ptr<std::function<void(EventType)>>> v = std::any_cast<std::vector<std::shared_ptr<std::function<void(EventType)>>>>(_mEventHandlers[packerHeaderId]);
            v.erase(std::remove(v.begin(), v.end(), handler), v.end());
            _mEventHandlers[packerHeaderId] = v;
        }

        /**
         * @brief When receiving a packet, this method will be called to trigger the handler of the event.
         *
         * @tparam EventType
         * @param packerHeaderId
         * @param data
         */
        template <class EventType>
        void triggerHandler(const uint32_t &packerHeaderId,
                            const std::vector<std::byte> &data)
        {
            auto it = _mEventHandlers.find(packerHeaderId);

            if (it == this->_mEventHandlers.end())
            {
                return;
            }

            std::vector<std::shared_ptr<std::function<void(EventType)>>> v = std::any_cast<std::vector<std::shared_ptr<std::function<void(EventType)>>>>(this->_mEventHandlers[packerHeaderId]);

            EventType e = deserializeData<EventType>(data);

            for (auto &fct_ptr : v)
            {
                fct_ptr.get()->operator()(e);
            }
        }

        /**
         * @brief When sending a packet, this method will be called to deserialize the data.
         * @tparam EventType The type of the event.
         * @param data The data to deserialize.
         * @return The deserialized data as an event.
         */
        template <class EventType>
        EventType deserializeData(const std::vector<std::byte> &data)
        {
            EventType e;
            memcpy(&e, data.data(), sizeof(EventType));
            return (e);
        }

        /**
         * @brief When sending a packet, this method will be called to serialize the data.
         * @tparam EventType The type of the event.
         * @param e The event to serialize.
         * @return The serialized data.
         */
        template <class EventType>
        std::vector<std::byte> serializeData(const EventType &e)
        {
            std::vector<std::byte> v;
            v.resize(sizeof(EventType));
            memcpy(v.data(), &e, sizeof(EventType));
            return (v);
        }

        /**
         * @brief Count the number of handlers for the event.
         * @tparam EventType The type of the event.
         * @param packerHeaderId The id of the packet.
         * @return The number of handlers.
         */
        template<typename EventType>
        int countHandler(const uint32_t &packerHeaderId)
        {
            auto it = _mEventHandlers.find(packerHeaderId);

            if (it == this->_mEventHandlers.end())
            {
                return (0);
            }

            std::vector<std::shared_ptr<std::function<void(EventType)>>> v = std::any_cast<std::vector<std::shared_ptr<std::function<void(EventType)>>>>(this->_mEventHandlers[packerHeaderId]);

            return (v.size());
        }

    private:
        std::map<uint32_t, std::any> _mEventHandlers;
    };

