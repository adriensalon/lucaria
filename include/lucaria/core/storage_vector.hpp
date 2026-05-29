// #pragma once

// #include <lucaria/bin/types_containers.hpp>
// #include <lucaria/bin/types_math.hpp>

// template <typename T, std::size_t PageSize = 256, typename Allocator = std::allocator<T>>
// struct container_page_vector_cpu {

//     using value_type = T;
//     using size_type = std::size_t;
//     using allocator_type = Allocator;

// private:
//     struct slot {
//         alignas(T) std::byte raw[sizeof(T)];
//         bool alive { false };

//         T* ptr() noexcept
//         {
//             return std::launder(reinterpret_cast<T*>(raw));
//         }

//         const T* ptr() const noexcept
//         {
//             return std::launder(reinterpret_cast<const T*>(raw));
//         }
//     };

//     struct page {
//         slot slots[PageSize];
//         uint32 used {}; // high-water slots in this page
//         uint32 live {}; // alive slots in this page
//     };

//     using page_allocator_type =
//         typename std::allocator_traits<Allocator>::template rebind_alloc<page>;
//     using page_alloc_traits = std::allocator_traits<page_allocator_type>;

// public:
//     explicit container_page_vector_cpu(const Allocator& allocator = Allocator {})
//         : allocator_ { allocator }
//     {
//     }

//     ~container_page_vector_cpu()
//     {
//         clear();
//         release_all_pages();
//     }

//     container_page_vector_cpu(const container_page_vector_cpu&) = delete;
//     container_page_vector_cpu& operator=(const container_page_vector_cpu&) = delete;

//     container_page_vector_cpu(container_page_vector_cpu&& other) noexcept
//         : allocator_ { std::move(other.allocator_) }
//         , pages_ { std::move(other.pages_) }
//         , free_pages_ { std::move(other.free_pages_) }
//         , live_size_ { other.live_size_ }
//         , slot_size_ { other.slot_size_ }
//     {
//         other.live_size_ = 0;
//         other.slot_size_ = 0;
//     }

//     container_page_vector_cpu& operator=(container_page_vector_cpu&& other) noexcept
//     {
//         if (this == &other) {
//             return *this;
//         }

//         clear();
//         release_all_pages();

//         allocator_ = std::move(other.allocator_);
//         pages_ = std::move(other.pages_);
//         free_pages_ = std::move(other.free_pages_);
//         live_size_ = other.live_size_;
//         slot_size_ = other.slot_size_;

//         other.live_size_ = 0;
//         other.slot_size_ = 0;

//         return *this;
//     }

//     [[nodiscard]] size_type size() const noexcept
//     {
//         return live_size_;
//     }

//     [[nodiscard]] bool empty() const noexcept
//     {
//         return live_size_ == 0;
//     }

//     [[nodiscard]] size_type slot_size() const noexcept
//     {
//         return slot_size_;
//     }

//     [[nodiscard]] static constexpr size_type page_size() noexcept
//     {
//         return PageSize;
//     }

//     template <typename... Args>
//     T& emplace_back(Args&&... args)
//     {
//         page* pg = ensure_back_page();

//         const auto offset = pg->used++;
//         slot& s = pg->slots[offset];

//         assert(!s.alive);

//         std::construct_at(s.ptr(), std::forward<Args>(args)...);
//         s.alive = true;

//         ++pg->live;
//         ++live_size_;
//         ++slot_size_;

//         return *s.ptr();
//     }

//     void push_back(const T& value)
//     {
//         emplace_back(value);
//     }

//     void push_back(T&& value)
//     {
//         emplace_back(std::move(value));
//     }

//     void pop_back()
//     {
//         assert(slot_size_ > 0);

//         while (!pages_.empty()) {
//             page* pg = pages_.back();
//             if (pg->used == 0) {
//                 release_back_page();
//                 continue;
//             }

//             const auto offset = pg->used - 1u;
//             slot& s = pg->slots[offset];
//             --pg->used;
//             --slot_size_;
//             if (s.alive) {
//                 std::destroy_at(s.ptr());
//                 s.alive = false;
//                 --pg->live;
//                 --live_size_;
//             }

//             if (pg->used == 0) {
//                 release_back_page();
//             }

//             return;
//         }

//         assert(false && "container_page_vector_cpu corrupted: slot_size_ > 0 but no pages");
//     }

//     void clear() noexcept
//     {
//         for (page* pg : pages_) {
//             for (uint32 i = 0; i < pg->used; ++i) {
//                 slot& s = pg->slots[i];

//                 if (s.alive) {
//                     std::destroy_at(s.ptr());
//                     s.alive = false;
//                 }
//             }

//             pg->used = 0;
//             pg->live = 0;

//             free_pages_.push_back(pg);
//         }

//         pages_.clear();
//         live_size_ = 0;
//         slot_size_ = 0;
//     }

//     T& operator[](size_type index) noexcept
//     {
//         const auto page_index = index / PageSize;
//         const auto offset = index % PageSize;

//         assert(page_index < pages_.size());

//         slot& s = pages_[page_index]->slots[offset];
//         assert(s.alive);

//         return *s.ptr();
//     }

//     const T& operator[](size_type index) const noexcept
//     {
//         const auto page_index = index / PageSize;
//         const auto offset = index % PageSize;

//         assert(page_index < pages_.size());

//         const slot& s = pages_[page_index]->slots[offset];
//         assert(s.alive);

//         return *s.ptr();
//     }

//     template <typename Fn>
//     void each_page(Fn&& fn)
//     {
//         for (page* pg : pages_) {
//             if (pg->live == 0) {
//                 continue;
//             }

//             fn(page_view { pg });
//         }
//     }

//     template <typename Fn>
//     void each_page(Fn&& fn) const
//     {
//         for (const page* pg : pages_) {
//             if (pg->live == 0) {
//                 continue;
//             }

//             fn(const_page_view { pg });
//         }
//     }

// private:
//     struct page_view {
//         page* pg {};

//         [[nodiscard]] uint32 used() const noexcept
//         {
//             return pg->used;
//         }

//         [[nodiscard]] uint32 live() const noexcept
//         {
//             return pg->live;
//         }

//         [[nodiscard]] bool alive(uint32 i) const noexcept
//         {
//             assert(i < pg->used);
//             return pg->slots[i].alive;
//         }

//         T& operator[](uint32 i) const noexcept
//         {
//             assert(i < pg->used);
//             assert(pg->slots[i].alive);
//             return *pg->slots[i].ptr();
//         }

//         template <typename Fn>
//         void each(Fn&& fn) const
//         {
//             for (uint32 i = 0; i < pg->used; ++i) {
//                 if (pg->slots[i].alive) {
//                     fn(i, *pg->slots[i].ptr());
//                 }
//             }
//         }
//     };

//     struct const_page_view {
//         const page* pg {};

//         [[nodiscard]] uint32 used() const noexcept
//         {
//             return pg->used;
//         }

//         [[nodiscard]] uint32 live() const noexcept
//         {
//             return pg->live;
//         }

//         [[nodiscard]] bool alive(uint32 i) const noexcept
//         {
//             assert(i < pg->used);
//             return pg->slots[i].alive;
//         }

//         const T& operator[](uint32 i) const noexcept
//         {
//             assert(i < pg->used);
//             assert(pg->slots[i].alive);
//             return *pg->slots[i].ptr();
//         }

//         template <typename Fn>
//         void each(Fn&& fn) const
//         {
//             for (uint32 i = 0; i < pg->used; ++i) {
//                 if (pg->slots[i].alive) {
//                     fn(i, *pg->slots[i].ptr());
//                 }
//             }
//         }
//     };

// private:
//     page* ensure_back_page()
//     {
//         if (pages_.empty() || pages_.back()->used == PageSize) {
//             pages_.push_back(acquire_page());
//         }

//         return pages_.back();
//     }

//     page* acquire_page()
//     {
//         page* pg {};

//         if (!free_pages_.empty()) {
//             pg = free_pages_.back();
//             free_pages_.pop_back();
//         } else {
//             pg = page_alloc_traits::allocate(allocator_, 1);
//             page_alloc_traits::construct(allocator_, pg);
//         }

//         pg->used = 0;
//         pg->live = 0;

//         for (auto& s : pg->slots) {
//             s.alive = false;
//         }

//         return pg;
//     }

//     void release_back_page()
//     {
//         page* pg = pages_.back();
//         pages_.pop_back();

//         pg->used = 0;
//         pg->live = 0;

//         free_pages_.push_back(pg);
//     }

//     void release_all_pages() noexcept
//     {
//         for (page* pg : pages_) {
//             page_alloc_traits::destroy(allocator_, pg);
//             page_alloc_traits::deallocate(allocator_, pg, 1);
//         }

//         for (page* pg : free_pages_) {
//             page_alloc_traits::destroy(allocator_, pg);
//             page_alloc_traits::deallocate(allocator_, pg, 1);
//         }

//         pages_.clear();
//         free_pages_.clear();
//     }

// private:
//     page_allocator_type allocator_;
//     std::vector<page*> pages_;
//     std::vector<page*> free_pages_;
//     size_type live_size_ {};
//     size_type slot_size_ {};
// };