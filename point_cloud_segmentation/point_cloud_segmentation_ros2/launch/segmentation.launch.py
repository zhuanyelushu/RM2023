import os
from ament_index_python.packages import get_package_share_directory
from launch import LaunchDescription
from launch_ros.actions import Node

def generate_launch_description():
    bringup_dir = get_package_share_directory('point_cloud_segmentation_ros2')
    param_file = os.path.join(bringup_dir, 'launch', 'segmentation_params.yaml')

    node_start_cmd = Node(
        package='point_cloud_segmentation_ros2',
        executable='point_cloud_segmentation',
        parameters=[param_file],
        output='screen'
    )

    ld = LaunchDescription()

    ld.add_action(node_start_cmd)
    return ld