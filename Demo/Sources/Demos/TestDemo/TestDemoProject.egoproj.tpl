<Project Name="TestDemoProject">
    <AssetDirectories>
        <AssetDirectory Path="@TPL_TEST_DEMO_ASSETS_PATH@" />
    </AssetDirectories>
    <PluginDirectories>
        <PluginDirectory Path="@TPL_TEST_DEMO_PLUGINS_PATH@" />
        <PluginDirectory Path="@TPL_TEST_DEMO_GAME_PLUGINS_PATH@" />
    </PluginDirectories>
    <EngineLogic>
        <Plugin Name="TestDemo" />
    </EngineLogic>
</Project>
