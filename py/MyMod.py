import os
import logging
from pathlib import Path
from typing import Optional

class ClangeMeMod:
    _instance = None

    @staticmethod
    def instance():
        if ClangeMeMod._instance is None:
            ClangeMeMod._instance = ClangeMeMod()
        return ClangeMeMod._instance

    def __init__(self):
        self.mSelf = ll.mod.NativeMod.current()
        self.mConfigFile: Optional[ConfigFile] = None
        self.mConfig = None

    def getSelf(self):
        return self.mSelf

    def load(self) -> bool:
        self_ref = self.getSelf()
        logger = self_ref.getLogger()
        logger.debug("Loading...")

        try:
            os.makedirs(self_ref.getDataDir(), exist_ok=True)
        except OSError as e:
            logger.error(f"Failed to create data directory {self_ref.getDataDir()}: {e}")
            return False

        try:
            os.makedirs(self_ref.getConfigDir(), exist_ok=True)
        except OSError as e:
            logger.error(f"Failed to create config directory {self_ref.getConfigDir()}: {e}")
            return False

        self.mConfigFile = ConfigFile()
        if not self.mConfigFile.load():
            logger.warning("Failed to load typed config")
            return False

        self.mConfig = self.mConfigFile.value()

        logger.info(f"Loaded {self_ref.getName()} from {self_ref.getModDir()}")
        return True

    def enable(self) -> bool:
        self_ref = self.getSelf()
        logger = self_ref.getLogger()
        logger.debug("Enabling...")
        if not self.mConfig.enabled:
            logger.info("clange_me is disabled by config")
            return True

        logger.info(f"Config message: {self.mConfig.message}")
        return True

    def disable(self) -> bool:
        self.getSelf().getLogger().debug("Disabling...")
        # Undo enable-time state here.
        return True

    def unload(self) -> bool:
        self.getSelf().getLogger().debug("Unloading...")
        # Release load-time resources here.
        self.mConfigFile = None
        return True


# Placeholder classes for ll.mod.NativeMod and ConfigFile to illustrate structure
class ll:
    class mod:
        class NativeMod:
            @staticmethod
            def current():
                return NativeMod()

            def getLogger(self):
                return logging.getLogger("ClangeMeMod")

            def getDataDir(self):
                return Path("./data")

            def getConfigDir(self):
                return Path("./config")

            def getName(self):
                return "ClangeMeMod"

            def getModDir(self):
                return Path("./moddir")

class ConfigFile:
    def load(self) -> bool:
        # Implement loading logic here
        return True

    def value(self):
        # Return a config object with 'enabled' and 'message' attributes
        class Config:
            enabled = True
            message = "Hello from config"
        return Config()
