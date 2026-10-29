/*
*   Copyright (C) 2026 Roan Bukaci
*   SPDX-License-Identifier: GPL-3.0
*
*   Implementation of the serializable concept and supporting tools
*/
export module util:serializable;
import :core_utils;
import std;

export template <typename S>
concept ByteSink = requires(S s, std::span<std::byte const> data) {
    { s.append(data) } -> std::same_as<void>;
};

template <typename T, typename S>
concept MemberSerializable = ByteSink<S> && requires(T const& a, S& s)
{
    { a.serialize(s) } -> std::same_as<void>;
};

template <typename T, typename S>
concept FreeSerializable = ByteSink<S> && requires(T const& a, S& s)
{
    { serialize(a, s) } -> std::same_as<void>;
};

export template <typename T, typename S>
concept SerializableTo = MemberSerializable<T, S> || FreeSerializable<T, S>;

/**
 * @brief
 * @tparam T Shall fulfill the SerializableTo concept
 * @tparam S Shall fulfill the ByteSink concept
 * @param obj The object to serialize
 * @param sink Processes the serialization, library provided or user made
 */
export template <typename T, ByteSink S>
auto serialize_to(T const& obj, S& sink) -> void {
    if constexpr (MemberSerializable<T, S>) {
        obj.serialize(sink);
    } else if constexpr (FreeSerializable<T, S>) {
        serialize(obj, sink);
    } else {
        static_assert(SerializableTo<T, S>, "T is not serializable to this sink");
    }
}

export namespace byte_sinks
{
    /**
     * @brief Sink to a std::ostream
     */
    struct StreamSink
    {
        std::ostream& out;
        std::array<std::byte, 64_KiB> buffer{};
        std::size_t used{};
        void append(std::span<std::byte const> data)
        {
            if (data.size() <= buffer.size() - used)
            {
                std::ranges::copy(data, buffer.begin() + used);
                used += data.size();
            }
            else
            {
                auto written_bytes{0UZ};
                while (written_bytes != buffer.size())
                {
                    auto const bytes_to_write = std::clamp(buffer.size() - used, 0UZ, data.size() - written_bytes);

                    std::ranges::copy(data.begin() + written_bytes,
                                        data.begin() + written_bytes + bytes_to_write,
                                      buffer.begin() + used);

                    written_bytes += bytes_to_write;
                    flush();
                }
            }
        }
        ~StreamSink() { flush(); }

        auto flush() noexcept(false) -> void
        {
            out.write(reinterpret_cast<const char*>(buffer.data()), used);
            used = 0;
        }
    };

    /**
     * @brief Sink to a std::vector
     */
    template <typename Alloc = std::allocator<std::byte>>
    struct VectorSink
    {
        std::vector<std::byte, Alloc>& output;
        void append(std::span<std::byte const> data)
        {
            output.append_range(data);
        }
    };
}

/**
 * @tparam T Shall fulfill the SerializableTo concept
 * @param[in] obj The object to serialize
 * @param os The file stream where the serialized \p obj is written to
 */
export template<typename T>
void write_to_disk(T const& obj, std::ofstream& os)
{
    byte_sinks::StreamSink sink{.out = os};
    serialize_to(obj, sink);
}