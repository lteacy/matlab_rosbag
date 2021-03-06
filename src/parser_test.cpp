#include "gtest/gtest.h"

#include <boost/scoped_ptr.hpp>

#include "parser.hpp"

using namespace std;
TEST(ROSType, builtin) {
  ROSType f;
  f.populate("string");

  EXPECT_EQ(string("string"), f.name);
  EXPECT_EQ(string("string"), f.base_type);
  EXPECT_EQ(string("string"), f.msg_name);
  EXPECT_EQ(string(""), f.pkg_name);
  EXPECT_FALSE(f.is_array);
  EXPECT_TRUE(f.is_qualified);
  EXPECT_FALSE(f.is_array);
  EXPECT_EQ(1, f.array_size);
  EXPECT_EQ(-1, f.type_size);
}

TEST(ROSType, unqualified_array) {
  ROSType f;
  f.populate("foo[40]");

  EXPECT_EQ(string("foo[40]"), f.name);
  EXPECT_EQ(string("foo"), f.base_type);
  EXPECT_EQ(string("foo"), f.msg_name);
  EXPECT_EQ(string(""), f.pkg_name);
  EXPECT_TRUE(f.is_array);
  EXPECT_FALSE(f.is_qualified);
  EXPECT_TRUE(f.is_array);
  EXPECT_EQ(40, f.array_size);
}

TEST(ROSType, builtin_fixedlen_array) {
  ROSType f;
  f.populate("float64[32]");

  EXPECT_EQ(string("float64[32]"), f.name);
  EXPECT_EQ(string("float64"), f.base_type);
  EXPECT_EQ(string("float64"), f.msg_name);
  EXPECT_EQ(string(""), f.pkg_name);
  EXPECT_TRUE(f.is_array);
  EXPECT_TRUE(f.is_qualified);
  EXPECT_TRUE(f.is_array);
  EXPECT_EQ(32, f.array_size);
  EXPECT_EQ(8, f.type_size);
}


TEST(ROSType, qualified_array) {
  ROSType f;
  f.populate("geometry_msgs/Pose[]");

  EXPECT_EQ(string("geometry_msgs/Pose[]"), f.name);
  EXPECT_EQ(string("geometry_msgs/Pose"), f.base_type);
  EXPECT_EQ(string("Pose"), f.msg_name);
  EXPECT_EQ(string("geometry_msgs"), f.pkg_name);
  EXPECT_TRUE(f.is_array);
  EXPECT_TRUE(f.is_qualified);
  EXPECT_TRUE(f.is_array);
  EXPECT_EQ(-1, f.array_size);
}

TEST(ROSTypeMap, bad_def) {
  ROSTypeMap rtm;
  string def("foo field1\n"
             "==\n"
             "MSG: asdf/foo\n"
             "uint8 field2\n"
             "==\n"
             "MSG: qwerty/foo\n"
             "uint8 field3\n"
             "\n");
  ASSERT_THROW(rtm.populate(def), invalid_argument);
}

TEST(ROSMessageFields, parse_quaternion_def) {
  ROSMessageFields mt;
  string
    def("MSG: geometry_msgs/Quaternion\n"
        "# This represents an orientation in free space in quaternion form.\n"
        "\n"
        "float64 x\n"
        "float64 y\n"
        "float64 z\n"
        "float64 w\n");
  mt.populate(def);
  EXPECT_EQ(string("geometry_msgs/Quaternion"), mt.type().name);
  ASSERT_EQ(4, mt.nfields());
  EXPECT_EQ(string("float64"), mt.at(0).type.name);
  EXPECT_EQ(string("float64"), mt.at(1).type.name);
  EXPECT_EQ(string("float64"), mt.at(2).type.name);
  EXPECT_EQ(string("float64"), mt.at(3).type.name);
  EXPECT_EQ(string("x"), mt.at(0).name);
  EXPECT_EQ(string("y"), mt.at(1).name);
  EXPECT_EQ(string("z"), mt.at(2).name);
  EXPECT_EQ(string("w"), mt.at(3).name);
  EXPECT_FALSE(mt.at(0).constant);
  EXPECT_FALSE(mt.at(1).constant);
  EXPECT_FALSE(mt.at(2).constant);
  EXPECT_FALSE(mt.at(3).constant);
}

TEST(ROSMessageFields, parse_comments) {
  ROSMessageFields mt;
  string
    def("MSG: geometry_msgs/Quaternion\n"
        "\n"
        "          # I'm a comment after whitespace\n"
        "float64 x # I'm an end of line comment float64 y\n"
        "float64 z\n"
        );
  mt.populate(def);
  EXPECT_EQ(string("geometry_msgs/Quaternion"), mt.type().name);
  ASSERT_EQ(2, mt.nfields());
  EXPECT_EQ(string("float64"), mt.at(0).type.name);
  EXPECT_EQ(string("float64"), mt.at(1).type.name);
  EXPECT_EQ(string("x"), mt.at(0).name);
  EXPECT_EQ(string("z"), mt.at(1).name);
  EXPECT_FALSE(mt.at(0).constant);
  EXPECT_FALSE(mt.at(1).constant);
}

TEST(ROSMessageFields, constant_uint8) {
  ROSMessageFields fields;
  string def("uint8 a = 1\n");
  fields.populate(def);
  ASSERT_EQ(1, fields.nfields());
  EXPECT_EQ(string("a"), fields.at(0).name);
  EXPECT_EQ(string("uint8"), fields.at(0).type.base_type);
  EXPECT_TRUE(fields.at(0).constant);
  EXPECT_EQ(string("1"), fields.at(0).value);
  EXPECT_EQ(1, fields.at(0).bytes.size());
  EXPECT_EQ(1, fields.at(0).bytes[0]);
}

TEST(ROSMessageFields, constant_string) {
  ROSMessageFields fields;
  string def("string msg = ab9\n");
  fields.populate(def);
  ASSERT_EQ(1, fields.nfields());
  EXPECT_EQ(string("msg"), fields.at(0).name);
  EXPECT_EQ(string("string"), fields.at(0).type.base_type);
  EXPECT_TRUE(fields.at(0).constant);
  EXPECT_EQ(string("ab9"), fields.at(0).value);

  uint8_t bytes[] = {'a', 'b', '9'};
  EXPECT_EQ(3, fields.at(0).bytes.size());
  EXPECT_EQ(bytes[0], fields.at(0).bytes[0]);
  EXPECT_EQ(bytes[1], fields.at(0).bytes[1]);
  EXPECT_EQ(bytes[2], fields.at(0).bytes[2]);
}

TEST(ROSMessageFields, constant_comments) {
  ROSMessageFields fields;
  string def(
"string str=  this string has a # comment in it  \n"
"string str2 = this string has \"quotes\" and \\slashes\\ in it\n"
"float64 a=64.0 # numeric comment\n");
  fields.populate(def);
  ASSERT_EQ(3, fields.nfields());
  EXPECT_EQ(string("str"), fields.at(0).name);
  EXPECT_EQ(string("string"), fields.at(0).type.base_type);
  EXPECT_TRUE(fields.at(0).constant);
  EXPECT_EQ(string("this string has a # comment in it"), fields.at(0).value);

  EXPECT_EQ(string("str2"), fields.at(1).name);
  EXPECT_EQ(string("string"), fields.at(1).type.base_type);
  EXPECT_TRUE(fields.at(1).constant);
  EXPECT_EQ(string("this string has \"quotes\" and \\slashes\\ in it"),
            fields.at(1).value);

  EXPECT_EQ(string("a"), fields.at(2).name);
  EXPECT_EQ(string("float64"), fields.at(2).type.base_type);
  EXPECT_TRUE(fields.at(2).constant);
  EXPECT_EQ(string("64.0"), fields.at(2).value);
}

TEST(ROSTypeMap, parse_pose_def) {
  string
    def(
"# A representation of pose in free space, composed of postion and orientation. \n"
"Point position\n"
"Quaternion orientation\n"
"\n"
"================================================================================\n"
"MSG: geometry_msgs/Point\n"
"# This contains the position of a point in free space\n"
"float64 x\n"
"float64 y\n"
"float64 z\n"
"\n"
"================================================================================\n"
"MSG: geometry_msgs/Quaternion\n"
"# This represents an orientation in free space in quaternion form.\n"
"\n"
"float64 x\n"
"float64 y\n"
"float64 z\n"
"float64 w");

  ROSTypeMap rtm;
  rtm.populate(def);

  const ROSMessageFields *type;
  ASSERT_TRUE((type = rtm.getMsgFields(string("0-root"))) != NULL);
  EXPECT_EQ(2, type->nfields());
  EXPECT_EQ(string("position"), type->at(0).name);
  EXPECT_EQ(string("geometry_msgs/Point"), type->at(0).type.base_type);

  ASSERT_TRUE((type = rtm.getMsgFields(string("geometry_msgs/Quaternion"))) != NULL);
  EXPECT_EQ(4, type->nfields());

  ASSERT_TRUE((type = rtm.getMsgFields(string("geometry_msgs/Point"))) != NULL);
  EXPECT_EQ(3, type->nfields());
}

TEST(ROSTypeMap, constant) {
  ROSTypeMap rtm;
  string def("string constant = as\n\n");
  rtm.populate(def);

  const ROSMessageFields *msg_type;
  ASSERT_TRUE((msg_type = rtm.getMsgFields(string("0-root"))) != NULL);
  ROSMessage msg(msg_type->type());
  msg.populate(rtm, NULL, 0);

  const ROSMessage::Field &constant = msg.lookupField("constant");
  EXPECT_EQ(1, constant.size());
  EXPECT_EQ(1, constant.at(0).bytes().size());
  const vector<uint8_t>& bytes = constant.at(0).bytes().at(0);
  EXPECT_EQ(2, bytes.size());
  EXPECT_EQ('a', bytes[0]);
  EXPECT_EQ('s', bytes[1]);

}


TEST(ROSTypeMap, single_string) {
  ROSTypeMap rtm;
  string def("string test\n\n");
  rtm.populate(def);

  const ROSMessageFields *msg_type;
  ASSERT_TRUE((msg_type = rtm.getMsgFields(string("0-root"))) != NULL);
  ROSMessage msg(msg_type->type());

  uint8_t bytes[] = {2, 0, 0, 0, 'h', 'i'};
  int beg = 0;
  msg.populate(rtm, bytes, &beg);

  const ROSMessage::Field &test = msg.lookupField(string("test"));
  ASSERT_EQ(1, test.size());
  EXPECT_EQ(beg, 6);
  EXPECT_EQ('h', test.at(0).bytes()[0][0]);
  EXPECT_EQ('i', test.at(0).bytes()[0][1]);
}

TEST(ROSTypeMap, varlen_array) {
  ROSTypeMap rtm;
  string def(
"Point[] points\n"
"\n"
"================================================================================\n"
"MSG: geometry_msgs/Point\n"
"uint8 x\n"
"int32 y\n");
  rtm.populate(def);

  const ROSMessageFields *msg_type;
  ASSERT_TRUE((msg_type = rtm.getMsgFields(string("0-root"))) != NULL);
  ROSMessage msg(msg_type->type());

  uint8_t bytes[] = {2, 0, 0, 0,
                     5, 1, 0, 0, 0,
                     6, 0, 1, 0, 1};
  int beg = 0;
  msg.populate(rtm, bytes, &beg);

  const ROSMessage::Field &points = msg.lookupField(string("points"));
  ASSERT_EQ(2, points.size());
  EXPECT_EQ(beg, 14);

  EXPECT_EQ(5, points.at(0).lookupField("x").at(0).bytes()[0][0]);
  EXPECT_EQ(1, points.at(0).lookupField("y").at(0).bytes()[0][0]);
  EXPECT_EQ(0, points.at(0).lookupField("y").at(0).bytes()[0][1]);
  EXPECT_EQ(0, points.at(0).lookupField("y").at(0).bytes()[0][2]);
  EXPECT_EQ(0, points.at(0).lookupField("y").at(0).bytes()[0][3]);

  EXPECT_EQ(6, points.at(1).lookupField("x").at(0).bytes()[0][0]);
  EXPECT_EQ(0, points.at(1).lookupField("y").at(0).bytes()[0][0]);
  EXPECT_EQ(1, points.at(1).lookupField("y").at(0).bytes()[0][1]);
  EXPECT_EQ(0, points.at(1).lookupField("y").at(0).bytes()[0][2]);
  EXPECT_EQ(1, points.at(1).lookupField("y").at(0).bytes()[0][3]);
}


// Run all the tests that were declared with TEST()
int main(int argc, char **argv){
    testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
