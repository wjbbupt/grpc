//
//
// Copyright 2015 gRPC authors.
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.
//
//

#include "src/core/transport/auth_context.h"

#include <string.h>

#include "absl/log/log.h"
#include "gmock/gmock.h"
#include "gtest/gtest.h"
#include "src/core/config/core_configuration.h"
#include "src/core/util/crash.h"
#include "src/core/util/ref_counted_ptr.h"
#include "src/core/util/string.h"
#include "test/core/test_util/test_config.h"

constexpr absl::string_view kProtocol = "baz";

TEST(AuthContextTest, EmptyContext) {
  grpc_core::RefCountedPtr<grpc_auth_context> ctx =
      grpc_core::MakeRefCounted<grpc_auth_context>(nullptr);
  grpc_auth_property_iterator it;

  LOG(INFO) << "test_empty_context";
  ASSERT_NE(ctx, nullptr);
  ASSERT_EQ(grpc_auth_context_peer_identity_property_name(ctx.get()), nullptr);
  it = grpc_auth_context_peer_identity(ctx.get());
  ASSERT_EQ(grpc_auth_property_iterator_next(&it), nullptr);
  it = grpc_auth_context_property_iterator(ctx.get());
  ASSERT_EQ(grpc_auth_property_iterator_next(&it), nullptr);
  it = grpc_auth_context_find_properties_by_name(ctx.get(), "foo");
  ASSERT_EQ(grpc_auth_property_iterator_next(&it), nullptr);
  ASSERT_EQ(grpc_auth_context_set_peer_identity_property_name(ctx.get(), "bar"),
            0);
  ASSERT_EQ(grpc_auth_context_peer_identity_property_name(ctx.get()), nullptr);
  ctx.reset(DEBUG_LOCATION, "test");
}

TEST(AuthContextTest, SimpleContext) {
  grpc_core::RefCountedPtr<grpc_auth_context> ctx =
      grpc_core::MakeRefCounted<grpc_auth_context>(nullptr);
  grpc_auth_property_iterator it;
  size_t i;

  LOG(INFO) << "test_simple_context";
  ASSERT_NE(ctx, nullptr);
  grpc_auth_context_add_cstring_property(ctx.get(), "name", "chapi");
  grpc_auth_context_add_cstring_property(ctx.get(), "name", "chapo");
  grpc_auth_context_add_cstring_property(ctx.get(), "foo", "bar");
  ASSERT_EQ(ctx->properties().count, 3);
  ASSERT_EQ(
      grpc_auth_context_set_peer_identity_property_name(ctx.get(), "name"), 1);

  ASSERT_STREQ(grpc_auth_context_peer_identity_property_name(ctx.get()),
               "name");
  it = grpc_auth_context_property_iterator(ctx.get());
  for (i = 0; i < ctx->properties().count; i++) {
    const grpc_auth_property* p = grpc_auth_property_iterator_next(&it);
    ASSERT_EQ(p, &ctx->properties().array[i]);
  }
  ASSERT_EQ(grpc_auth_property_iterator_next(&it), nullptr);

  it = grpc_auth_context_find_properties_by_name(ctx.get(), "foo");
  ASSERT_EQ(grpc_auth_property_iterator_next(&it), &ctx->properties().array[2]);
  ASSERT_EQ(grpc_auth_property_iterator_next(&it), nullptr);

  it = grpc_auth_context_peer_identity(ctx.get());
  ASSERT_EQ(grpc_auth_property_iterator_next(&it), &ctx->properties().array[0]);
  ASSERT_EQ(grpc_auth_property_iterator_next(&it), &ctx->properties().array[1]);
  ASSERT_EQ(grpc_auth_property_iterator_next(&it), nullptr);

  ctx.reset(DEBUG_LOCATION, "test");
}

TEST(AuthContextTest, ChainedContext) {
  grpc_core::RefCountedPtr<grpc_auth_context> chained =
      grpc_core::MakeRefCounted<grpc_auth_context>(nullptr);
  grpc_auth_context* chained_ptr = chained.get();
  grpc_core::RefCountedPtr<grpc_auth_context> ctx =
      grpc_core::MakeRefCounted<grpc_auth_context>(std::move(chained));

  grpc_auth_property_iterator it;
  size_t i;

  LOG(INFO) << "test_chained_context";
  grpc_auth_context_add_cstring_property(chained_ptr, "name", "padapo");
  grpc_auth_context_add_cstring_property(chained_ptr, "foo", "baz");
  grpc_auth_context_add_cstring_property(ctx.get(), "name", "chapi");
  grpc_auth_context_add_cstring_property(ctx.get(), "name", "chap0");
  grpc_auth_context_add_cstring_property(ctx.get(), "foo", "bar");
  ASSERT_EQ(
      grpc_auth_context_set_peer_identity_property_name(ctx.get(), "name"), 1);

  ASSERT_STREQ(grpc_auth_context_peer_identity_property_name(ctx.get()),
               "name");
  it = grpc_auth_context_property_iterator(ctx.get());
  for (i = 0; i < ctx->properties().count; i++) {
    const grpc_auth_property* p = grpc_auth_property_iterator_next(&it);
    ASSERT_EQ(p, &ctx->properties().array[i]);
  }
  for (i = 0; i < chained_ptr->properties().count; i++) {
    const grpc_auth_property* p = grpc_auth_property_iterator_next(&it);
    ASSERT_EQ(p, &chained_ptr->properties().array[i]);
  }
  ASSERT_EQ(grpc_auth_property_iterator_next(&it), nullptr);

  it = grpc_auth_context_find_properties_by_name(ctx.get(), "foo");
  ASSERT_EQ(grpc_auth_property_iterator_next(&it), &ctx->properties().array[2]);
  ASSERT_EQ(grpc_auth_property_iterator_next(&it),
            &chained_ptr->properties().array[1]);
  ASSERT_EQ(grpc_auth_property_iterator_next(&it), nullptr);

  it = grpc_auth_context_peer_identity(ctx.get());
  ASSERT_EQ(grpc_auth_property_iterator_next(&it), &ctx->properties().array[0]);
  ASSERT_EQ(grpc_auth_property_iterator_next(&it), &ctx->properties().array[1]);
  ASSERT_EQ(grpc_auth_property_iterator_next(&it),
            &chained_ptr->properties().array[0]);
  ASSERT_EQ(grpc_auth_property_iterator_next(&it), nullptr);

  ctx.reset(DEBUG_LOCATION, "test");
}

TEST(AuthContextTest, ContextWithExtension) {
  class SampleExtension : public grpc_auth_context::Extension {};
  grpc_core::RefCountedPtr<grpc_auth_context> ctx =
      grpc_core::MakeRefCounted<grpc_auth_context>(nullptr);
  // Just set the extension, the goal of this test is to catch any memory
  // leaks when context goes out of scope.
  ctx->set_extension(std::make_unique<SampleExtension>());
}

TEST(AuthContextTest, CompareAuthContextEqualProps) {
  // Setup two auth contexts with the same protocol and equal foo props
  grpc_core::RefCountedPtr<grpc_auth_context> ctx =
      grpc_core::MakeRefCounted<grpc_auth_context>(nullptr);
  ASSERT_NE(ctx, nullptr);
  grpc_auth_context_add_cstring_property(ctx.get(), "foo", "bar");
  grpc_core::RefCountedPtr<grpc_auth_context> ctx2 =
      grpc_core::MakeRefCounted<grpc_auth_context>(nullptr);
  ASSERT_NE(ctx2, nullptr);
  grpc_auth_context_add_cstring_property(ctx2.get(), "foo", "bar");
  ctx->set_protocol(kProtocol);
  ctx2->set_protocol(kProtocol);
  EXPECT_THAT(ctx->CompareAuthContext(ctx2.get()), ::testing::Optional(true));
  ctx.reset(DEBUG_LOCATION, "test");
  ctx2.reset(DEBUG_LOCATION, "test");
}

TEST(AuthContextTest, CompareAuthContextUnequalProps) {
  // Setup two auth contexts with  unequal foo props
  grpc_core::RefCountedPtr<grpc_auth_context> ctx =
      grpc_core::MakeRefCounted<grpc_auth_context>(nullptr);
  ASSERT_NE(ctx, nullptr);
  grpc_auth_context_add_cstring_property(ctx.get(), "foo", "bar");
  grpc_core::RefCountedPtr<grpc_auth_context> ctx2 =
      grpc_core::MakeRefCounted<grpc_auth_context>(nullptr);
  ASSERT_NE(ctx2, nullptr);
  grpc_auth_context_add_cstring_property(ctx2.get(), "foo", "baz");
  ctx->set_protocol(kProtocol);
  ctx2->set_protocol(kProtocol);
  EXPECT_THAT(ctx->CompareAuthContext(ctx2.get()), ::testing::Optional(false));
  ctx.reset(DEBUG_LOCATION, "test");
  ctx2.reset(DEBUG_LOCATION, "test");
}

TEST(AuthContextTest, CompareAuthContextNoProtocolReturnsOptional) {
  grpc_core::RefCountedPtr<grpc_auth_context> ctx =
      grpc_core::MakeRefCounted<grpc_auth_context>(nullptr);
  ASSERT_NE(ctx, nullptr);
  grpc_core::RefCountedPtr<grpc_auth_context> ctx2 =
      grpc_core::MakeRefCounted<grpc_auth_context>(nullptr);
  ASSERT_NE(ctx2, nullptr);
  EXPECT_THAT(ctx->CompareAuthContext(ctx2.get()), std::nullopt);
  ctx.reset(DEBUG_LOCATION, "test");
  ctx2.reset(DEBUG_LOCATION, "test");
}

TEST(AuthContextTest, CompareAuthContextUnsetReturnsOptional) {
  grpc_core::RefCountedPtr<grpc_auth_context> ctx =
      grpc_core::MakeRefCounted<grpc_auth_context>(nullptr);
  ASSERT_NE(ctx, nullptr);
  grpc_core::RefCountedPtr<grpc_auth_context> ctx2 =
      grpc_core::MakeRefCounted<grpc_auth_context>(nullptr);
  ASSERT_NE(ctx2, nullptr);
  constexpr absl::string_view kMissingProtocol = "NO_COMPARATOR_SET";
  ctx->set_protocol(kMissingProtocol);
  ctx2->set_protocol(kMissingProtocol);
  EXPECT_THAT(ctx->CompareAuthContext(ctx2.get()), std::nullopt);
  ctx.reset(DEBUG_LOCATION, "test");
  ctx2.reset(DEBUG_LOCATION, "test");
}

int main(int argc, char** argv) {
  grpc::testing::TestEnvironment env(&argc, argv);
  ::testing::InitGoogleTest(&argc, argv);
  // Set a custom comparison function
  grpc_core::CoreConfiguration::RegisterEphemeralBuilder(
      [&](grpc_core::CoreConfiguration::Builder* builder) {
        builder->auth_context_comparator_registry()->RegisterComparator(
            std::string(kProtocol),
            std::make_unique<absl::AnyInvocable<bool(
                const grpc_auth_context*, const grpc_auth_context*)>>(
                [&](const grpc_auth_context* one,
                    const grpc_auth_context* two) -> bool {
                  auto it1 =
                      grpc_auth_context_find_properties_by_name(one, "foo");
                  auto it2 =
                      grpc_auth_context_find_properties_by_name(two, "foo");
                  return strcmp(
                             grpc_auth_property_iterator_next(&it1)->value,
                             grpc_auth_property_iterator_next(&it2)->value) ==
                         0;
                }));
      });
  return RUN_ALL_TESTS();
}
