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
    template<class Allocator,
             class MemReclaimer >
    class brown_kcas
    {
    public:
        using alloc_t = std::size_t;
        using state_t = std::uintptr_t;
        
        enum class tag_type
        {
            NONE,
            RDCSS,
            KCAS
        };

        static const alloc_t S_NO_TAG = 0x0;
        static const alloc_t S_KCAS_TAG = 0x1;
        static const alloc_t S_RDCSS_TAG = 0x2;
        
        static const alloc_t S_THREAD_ID_SHIFT = 2;
        static const alloc_t S_THREAD_ID_MASK = (1 << 8) - 1;

        static const alloc_t S_SEQUENCE_SHIFT = 10;
        static const alloc_t S_SEQUENCE_MASK = (alloc_t(1) << 54) - 1;
        
        static const state_t UNDECIDED = 0;
        static const state_t SUCCESS = 1;
        static const state_t FAILED = 2;
    
    private:
        /**
         * @brief A class representing the
         * status of a given descriptor for
         * kCAS
         * 
         */
        class k_cas_descriptor_status
        {
        public:
            using descriptor = typename harris_kcas<Allocator, MemReclaimer>::k_cas_descriptor;

        private:
            std::unique_ptr<descriptor> _descriptor;
            state_t _status;
            state_t _sequence_number;
        
        public:
            explicit
            k_cas_descriptor_status() :
                _descriptor(std::make_unique<descriptor>()),
                _status(UNDECIDED),
                _sequence_number(0) {}
            
            explicit
            k_cas_descriptor_status(const state_t& status, 
                const state_t& sequence_number) :
                _descriptor(std::make_unique<descriptor>()),
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
            state_t _raw_bits;
        
        public:
            explicit
            tagged_pointer() : _raw_bits(0) {}

            explicit
            tagged_pointer(const state_t& raw_bits) : 
                _raw_bits(raw_bits)
            {
                assert(is_bits(*this));
            }

            explicit
            tagged_pointer(const state_t& tag_bits, 
                const state_t& thread_id,
                const state_t& sequence_number) : 
                _raw_bits(tag_bits | (thread_id << S_THREAD_ID_SHIFT) 
                    | (sequence_number << S_SEQUENCE_SHIFT)) {}

            tagged_pointer(const tagged_pointer&) = default;
            tagged_pointer &operator=(const tagged_pointer&) = default;

            ~tagged_pointer() {}

            static
            bool is_kcas(const tagged_pointer& tag_ptr)
            {
                return (tag_ptr._raw_bits & S_KCAS_TAG) == S_KCAS_TAG;
            }
            
            static
            bool is_rdcss(const tagged_pointer& tag_ptr)
            {
                return (tag_ptr._raw_bits & S_RDCSS_TAG) == S_RDCSS_TAG;
            }

            static
            bool is_bits(const tagged_pointer& tag_ptr)
            {
                return !(is_kcas(tag_ptr) || is_rdcss(tag_ptr));
            }
        };
    };
} // namespace crh

#endif // !CRH_BROWN_KCAS_HPP