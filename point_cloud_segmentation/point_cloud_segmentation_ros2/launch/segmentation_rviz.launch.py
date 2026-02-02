import os
from ament_index_python.packages import get_package_share_directory
from launch import LaunchDescription
from launch_ros.actions import Node

def generate_launch_description():
    bringup_dir = get_package_share_directory('point_cloud_segmentation_ros2')
    param_file = os.path.join(bringup_dir, 'launch', 'segmentation_params.yaml')
    rviz_config_file = os.path.join(bringup_dir, 'rviz', 'point_show.rviz')
    node_start_cmd = Node(
        package='point_cloud_segmentation_ros2',
        executable='point_cloud_segmentation',
        parameters=[param_file],
        output='screen'
    )
    start_rviz_cmd = Node(
        package='rviz2',
        executable='rviz2',
        arguments=['-d', rviz_config_file],
        output='screen'
    )

    ld = LaunchDescription()

    ld.add_action(node_start_cmd)
    ld.add_action(start_rviz_cmd)
    return ld