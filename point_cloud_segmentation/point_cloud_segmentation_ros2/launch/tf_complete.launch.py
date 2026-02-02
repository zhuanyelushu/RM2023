import os
from launch import LaunchDescription
from launch_ros.actions import Node
import math

def euler_to_quaternion(roll, pitch, yaw):
    """
    将欧拉角转换为四元数
    roll: 绕X轴 (rad)
    pitch: 绕Y轴 (rad) 
    yaw: 绕Z轴 (rad)
    """
    cy = math.cos(yaw * 0.5)
    sy = math.sin(yaw * 0.5)
    cp = math.cos(pitch * 0.5)
    sp = math.sin(pitch * 0.5)
    cr = math.cos(roll * 0.5)
    sr = math.sin(roll * 0.5)
    
    qw = cy * cp * cr + sy * sp * sr
    qx = cy * cp * sr - sy * sp * cr
    qy = sy * cp * sr + cy * sp * cr
    qz = sy * cp * cr - cy * sp * sr
    
    return qx, qy, qz, qw

def generate_launch_description():
    pitch_deg = 20 
    pitch_rad = math.radians(pitch_deg)
    
    qx, qy, qz, qw = euler_to_quaternion(0, pitch_rad, 0)
    
    static_tf_map_odom = Node(
        package='tf2_ros',
        executable='static_transform_publisher',
        name='static_tf_map_odom_init',
        arguments=['0', '0', '0', '0', '0', '0', 'map', 'odom'],
        output='screen'
    )
    
    static_tf_odom_base = Node(
        package='tf2_ros',
        executable='static_transform_publisher',
        name='static_tf_odom_base',
        arguments=['0', '0', '0', '0', '0', '0', 'odom', 'base_link'],
        output='screen'
    )
    
    static_tf_base_livox = Node(
        package='tf2_ros',
        executable='static_transform_publisher',
        name='static_tf_base_livox',
        arguments=[
            '0', '0', '0.16',  # 平移: x, y, z
            str(qx), str(qy), str(qz), str(qw),  # 四元数: x, y, z, w
            'base_link', 'livox_frame'
        ],
        output='screen'
    )
    
    test_tf_node = Node(
        package='tf2_ros',
        executable='tf2_echo',
        name='tf_test',
        arguments=['base_link', 'livox_frame'],
        output='screen'
    )
    
    return LaunchDescription([
        static_tf_map_odom,
        static_tf_odom_base,
        static_tf_base_livox,
        test_tf_node,  # 可以取消注释来测试
    ])