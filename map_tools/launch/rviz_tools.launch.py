import os
from ament_index_python.packages import get_package_share_directory
from launch import LaunchDescription
from launch.substitutions import LaunchConfiguration
from launch_ros.actions import Node
import launch_ros.actions
import launch.actions
import launch.events

def generate_launch_description():
    launch_file_dir = os.path.join(get_package_share_directory('map_tools'), 'launch')

    map_file = os.path.join(
        get_package_share_directory('map_tools'),
        'map',
        'map.yaml'
    )

    map_server_cmd = Node(
        package='nav2_map_server',
        executable='map_server',
        name='map_server',
        output='screen',
        parameters=[{'yaml_filename': map_file},
                    {'topic': 'map'},
                    {'frame_id': 'map'},
                    {'output': 'screen'},
                    {'use_sim_time': True}]
    )

    lifecycle_nodes = ['map_server']
    use_sim_time = True
    autostart = True

    start_lifecycle_manager_cmd = launch_ros.actions.Node(
            package='nav2_lifecycle_manager',
            executable='lifecycle_manager',
            name='lifecycle_manager',
            output='screen',
            emulate_tty=True,
            parameters=[{'use_sim_time': use_sim_time},
                        {'autostart': autostart},
                        {'node_names': lifecycle_nodes}])

    map_tf_cmd = launch_ros.actions.Node(
        package='tf2_ros',
        executable='static_transform_publisher',
        output='screen',
        arguments=['0', '0', '0', '0', '0', '0', 'map', 'base_link'])

    
    rviz_cmd = Node(
            package='rviz2',
            executable='rviz2',
            name='rviz2',
            arguments=['-d', [os.path.join(get_package_share_directory('map_tools'), 'rviz', 'map_tools.rviz')]]
        )
    rviz_tools_server_cmd = Node(
        package='map_tools',
        executable='rviz_tools_server',
        name='rviz_tools_server',
        output='screen',
        parameters=[{'use_sim_time': use_sim_time}]
    )

    # 创建一个事件处理程序，用于在Rviz2退出时发送一个事件
    # shutdown_handler = launch.actions.RegisterEventHandler(
    #     launch.event_handlers.OnProcessExit(
    #         target_action=rviz_cmd,
    #         on_exit=[
    #             launch.actions.LogInfo(
    #                 msg=(launch.substitutions.EnvironmentVariable(name='USER'),' closed the RViz2 window')
    #                 ),
    #             launch.actions.EmitEvent(event=launch.events.Shutdown(reason='Window closed'))
    #         ]
    #     )
    # )

    ld = LaunchDescription()

    ld.add_action(map_server_cmd)
    ld.add_action(start_lifecycle_manager_cmd)
    ld.add_action(map_tf_cmd)
    ld.add_action(rviz_cmd)
    ld.add_action(rviz_tools_server_cmd)
    # 添加事件处理程序到LaunchDescription
    # ld.add_action(shutdown_handler)

    return ld