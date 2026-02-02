from launch import LaunchDescription
from launch.actions import DeclareLaunchArgument
from launch.substitutions import LaunchConfiguration
from launch_ros.actions import Node

def generate_launch_description():
    return LaunchDescription([
        DeclareLaunchArgument(
            name='scanner',
            default_value='scanner',
            description='Namespace for sample topics'),
        Node(
            package='pointcloud_to_laserscan',
            executable='pointcloud_to_laserscan_node',
            remappings=[
                ('cloud_in', 'segmentation/obstacle'),
                ('scan', '/scan')
            ],
            parameters=[{
                'target_frame': 'livox_frame',
                'transform_tolerance': 0.1,
                'min_height': -0.5,
                'max_height': 0.5,
                'angle_min': -3.14159,
                'angle_max': 3.14159,
                'angle_increment': 0.0043,
                'scan_time': 0.1,
                'range_min': 0.45,
                'range_max': 10.0,
                'use_inf':  False,
                'inf_epsilon': 1.0,
            }],
            name='pointcloud_to_laserscan'
        )
    ])