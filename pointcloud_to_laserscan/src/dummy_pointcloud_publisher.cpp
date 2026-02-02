#include <memory>
#include <random>

#include "rclcpp/rclcpp.hpp"
#include "sensor_msgs/msg/point_cloud2.hpp"
#include "sensor_msgs/point_cloud2_iterator.hpp"

int main(int argc, char * argv[])
{
  rclcpp::init(argc, argv);

  auto node = std::make_shared<rclcpp::Node>("dummy_pointcloud_publisher");
  auto pub =
    node->create_publisher<sensor_msgs::msg::PointCloud2>("cloud", rclcpp::SensorDataQoS());

  sensor_msgs::msg::PointCloud2 dummy_cloud;
  sensor_msgs::PointCloud2Modifier modifier(dummy_cloud);
  modifier.setPointCloud2Fields(
    3,
    "x", 1, sensor_msgs::msg::PointField::FLOAT32,
    "y", 1, sensor_msgs::msg::PointField::FLOAT32,
    "z", 1, sensor_msgs::msg::PointField::FLOAT32);
  modifier.resize(node->declare_parameter("cloud_size", 100));
  std::mt19937 gen(node->declare_parameter("cloud_seed", 0));
  double extent = node->declare_parameter("cloud_extent", 10.0);
  std::uniform_real_distribution<float> distribution(-extent / 2, extent / 2);
  sensor_msgs::PointCloud2Iterator<float> it_x(dummy_cloud, "x");
  sensor_msgs::PointCloud2Iterator<float> it_y(dummy_cloud, "y");
  sensor_msgs::PointCloud2Iterator<float> it_z(dummy_cloud, "z");
  for (; it_x != it_x.end(); ++it_x, ++it_y, ++it_z) {
    *it_x = distribution(gen);
    *it_y = distribution(gen);
    *it_z = distribution(gen);
  }
  dummy_cloud.header.frame_id = node->declare_parameter("cloud_frame_id", "");

  rclcpp::executors::SingleThreadedExecutor executor;
  executor.add_node(node);
  rclcpp::Rate rate(1.0);
  while (rclcpp::ok()) {
    dummy_cloud.header.stamp = node->get_clock()->now();
    pub->publish(dummy_cloud);
    executor.spin_some();
    rate.sleep();
  }

  rclcpp::shutdown();
  return 0;
}