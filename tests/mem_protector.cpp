/******************************************************************************
 *                                                                            *
 * Copyright 2026  Nils Bosbach                                               *
 *                                                                            *
 * This software is licensed under the MIT license found in the               *
 * LICENSE file at the root directory of this source tree.                    *
 *                                                                            *
 ******************************************************************************/

#include "avp64/psp/mem_protector.h"

#include <gtest/gtest.h>
#include <gmock/gmock.h>
#include <cstdlib>
#include <cstring>

constexpr vcml::u64 TARGET_PAGE_SIZE = 4096;

class mock_core : public avp64::psp::mem_protector_if
{
public:
    virtual ~mock_core() override = default;

    virtual vcml::u64 page_size() override { return TARGET_PAGE_SIZE; }
    MOCK_METHOD(void, update_page, (vcml::u64 page_addr), (override));
};

TEST(avp64, mem_protector) {
    mock_core core;
    auto& mp = avp64::psp::mem_protector::instance();
    constexpr size_t target_page_cnt = 8;

    auto* test_pages = reinterpret_cast<vcml::u8*>(std::aligned_alloc(
        mwr::get_page_size(), target_page_cnt * TARGET_PAGE_SIZE));
    std::memset(test_pages, 0, target_page_cnt * TARGET_PAGE_SIZE);

    // register page
    mp.register_page(&core, 0, &test_pages[0]);

    // read locked page
    EXPECT_EQ(test_pages[1], 0);

    // write to locked page
    EXPECT_CALL(core, update_page(0)).Times(1);
    test_pages[1] = 1;
    EXPECT_EQ(test_pages[1], 1);

    // write again to page
    test_pages[1] = 2;
    EXPECT_EQ(test_pages[1], 2);

    // register and directly deregister page
    mp.register_page(&core, 0, &test_pages[0]);
    mp.deregister_page(&core, 0);

    // write to page
    test_pages[1] = 3;
    EXPECT_EQ(test_pages[1], 3);

    // register page
    mp.register_page(&core, 0, &test_pages[0]);

    // partially deregsiter page 0 --> page 0 should still be locked
    mp.deregister_pages(&core, 0, 1);

    // write to page zero
    EXPECT_CALL(core, update_page(0)).Times(1);
    test_pages[1] = 4;
    EXPECT_EQ(test_pages[1], 4);

    // register pages
    for (size_t i = 0; i <= 2; ++i)
        mp.register_page(&core, i * TARGET_PAGE_SIZE,
                         &test_pages[i * TARGET_PAGE_SIZE]);

    if (mwr::get_page_size() == TARGET_PAGE_SIZE) {
        // partially deregsiter pages
        //  0..TARGET_PAGE_SIZE-1:                    locked
        //  TARGET_PAGE_SIZE..2*TARGET_PAGE_SIZE-1:   unlocked
        //  2*TARGET_PAGE_SIZE..3*TARGET_PAGE_SIZE-1: locked
        mp.deregister_pages(&core, 1, 2 * TARGET_PAGE_SIZE - 1);

        test_pages[TARGET_PAGE_SIZE] = 5;
        EXPECT_EQ(test_pages[TARGET_PAGE_SIZE], 5);

        EXPECT_CALL(core, update_page(0)).Times(1);
        test_pages[1] = 6;
        EXPECT_EQ(test_pages[1], 6);

        EXPECT_CALL(core, update_page(2 * TARGET_PAGE_SIZE)).Times(1);
        test_pages[2 * TARGET_PAGE_SIZE] = 7;
        EXPECT_EQ(test_pages[2 * TARGET_PAGE_SIZE], 7);
    }

    std::free(test_pages);
}
