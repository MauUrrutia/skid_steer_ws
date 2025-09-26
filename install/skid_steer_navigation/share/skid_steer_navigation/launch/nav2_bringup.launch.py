from launch import LaunchDescription
from launch.actions import IncludeLaunchDescription, DeclareLaunchArgument, ExecuteProcess
from launch.launch_description_sources import PythonLaunchDescriptionSource
from launch.substitutions import PathJoinSubstitution
from launch_ros.actions import Node
from launch_ros.substitutions import FindPackageShare
import os

def generate_launch_description():
    # Ruta al archivo YAML de la máscara de Keepout Zones
    keepout_mask_yaml = PathJoinSubstitution([
        FindPackageShare('skid_steer_navigation'),
        'config',
        'keepout_mask.yaml'
    ])

    return LaunchDescription([
        # ===== 1. Nodos para Keepout Zones =====
        # Servidor de la máscara (publica la imagen de zonas prohibidas)
        # Node(
        #     package='nav2_map_server',
        #     executable='map_server',
        #     name='filter_mask_server',
        #     output='screen',
        #     parameters=[{
        #         'use_sim_time': True,
        #         'yaml_filename': keepout_mask_yaml,
        #         'topic_name': '/keepout_filter_mask'
        #     }]
        # ),

        # Servidor de información del filtro (metadata de las zonas)
        # Node(
        #     package='nav2_map_server',
        #     executable='costmap_filter_info_server',
        #     name='costmap_filter_info_server',
        #     output='screen',
        #     parameters=[{
        #         'use_sim_time': True,
        #         'filter_info_topic': '/costmap_filter_info',
        #         'mask_topic': '/keepout_filter_mask',
        #         'type': 0  # 0 = Keepout Zone
        #     }]
        # ),

        # ===== 2. Lifecycle Manager (incluye los nuevos nodos) =====
        # Node(
        #     package='nav2_lifecycle_manager',
        #     executable='lifecycle_manager',
        #     name='lifecycle_manager_filters',
        #     output='screen',
        #     parameters=[{
        #         'use_sim_time': True,
        #         'autostart': True,
        #         'node_names': [
        #             'filter_mask_server',
        #             'costmap_filter_info_server'
        #         ]
        #     }],
        #     remappings=[("/scan", "/ldlidar_node/scan")]
        # ),

        # ===== 3. Nav2 Principal =====
        IncludeLaunchDescription(
            PythonLaunchDescriptionSource(
                os.path.join(
                    '/opt/ros/humble/share/nav2_bringup/launch',
                    'navigation_launch.py'
                )
            ),
            launch_arguments={
                'use_sim_time': 'false',
                'params_file': PathJoinSubstitution([
                    FindPackageShare('skid_steer_navigation'),
                    'config',
                    'nav2_params.yaml'
                ])
            }.items()
        )

    ])
