#ifndef CRH_BROWN_KCAS_HPP
#define CRH_BROWN_KCAS_HPP

#include "harris_kcas.hpp"
namespace crh
{
    /**
     * @brief Implementation of 
     * modified kCAS algorithm as presented 
     * by Brown and Arbel-Raviv
     * 
     * @tparam Allocator An allocator policy
     * @tparam MemReclaimer A memory reclaimer policy
     */
    template< class Allocator,
              class MemReclaimer >
    class brown_kcas
    {
    public:
        using alloc_type = typename std::size_t;
        using state_type = typename std::uintptr_t;
        
        enum class tag_type
        {
            NONE,
            RDCSS,
            KCAS
        };

        static constexpr alloc_type S_NO_TAG = 0x0, S_KCAS_TAG = 0x1, S_RDCSS_TAG = 0x2;
        static constexpr alloc_type S_THREAD_ID_SHIFT = 2, S_THREAD_ID_MASK = (1 << 8) - 1;
        static constexpr alloc_type S_SEQUENCE_SHIFT = 10, S_SEQUENCE_MASK = (alloc_type(1) << 54) - 1;
        
        static constexpr state_type UNDECIDED = 0, SUCCESS = 1, FAILED = 2;
        
    private:
        /**
         * @brief A class representing the
         * status of a given descriptor for
         * kCAS
         * 
         */
        class k_cas_descriptor_status
        {
        private:
            state_type _sequence_number, _status;
            
        public:
            inline
            explicit
            k_cas_descriptor_status() :
                _status(UNDECIDED),
                _sequence_number(0) {}
            
            inline
            explicit
            k_cas_descriptor_status(const state_type& status, 
                const state_type& sequence_number) :
                _status(status), 
                _sequence_number(sequence_number) {}

            ~k_cas_descriptor_status() {}
        };

        /**
         * @brief A pointer with additional associated
         * data, in this case a raw bit count
         * 
         */
        class tagged_pointer
        {
        private:
            state_type _raw_bits;
        
        public:
            inline
            explicit
            tagged_pointer() : _raw_bits(0) {}

            inline
            explicit
            tagged_pointer(const state_type& raw_bits) : 
                _raw_bits(raw_bits)
            {
                static_assert(is_bits(*this));
            }

            explicit
            tagged_pointer(const state_type& tag_bits, 
                const state_type& thread_id,
                const state_type& sequence_number) : 
                _raw_bits(tag_bits | (thread_id << S_THREAD_ID_SHIFT) 
                    | (sequence_number << S_SEQUENCE_SHIFT)) {}

            tagged_pointer(const tagged_pointer&) = default;
            tagged_pointer &operator=(const tagged_pointer&) = default;

            ~tagged_pointer() {}
            
            static
            inline
            constexpr
            bool is_kcas(const tagged_pointer& tag_ptr) noexcept
            {
                return (tag_ptr._raw_bits & S_KCAS_TAG) == S_KCAS_TAG;
            }

            static
            inline
            constexpr
            bool is_rdcss(const tagged_pointer& tag_ptr) noexcept
            {
                return (tag_ptr._raw_bits & S_RDCSS_TAG) == S_RDCSS_TAG;
            }

            static
            inline
            constexpr
            bool is_bits(const tagged_pointer& tag_ptr) noexcept
            {
                return !(is_kcas(tag_ptr) || is_rdcss(tag_ptr));
            }
        };
    };
} // namespace crh

#endif // !CRH_BROWN_KCAS_HPP