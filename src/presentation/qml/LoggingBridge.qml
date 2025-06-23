import QtQuick 2.15
import QtQuick.Controls 2.15

QtObject {
    id: loggingBridge

    property var logger: null

    function debug(category, message) {
        if (logger) {
            logger.debug(category, message)
        } else {
            console.log("[DEBUG] [" + category + "] " + message)
        }
    }

    function info(category, message) {
        if (logger) {
            logger.info(category, message)
        } else {
            console.log("[INFO] [" + category + "] " + message)
        }
    }

    function warning(category, message) {
        if (logger) {
            logger.warning(category, message)
        } else {
            console.warn("[WARNING] [" + category + "] " + message)
        }
    }

    function critical(category, message) {
        if (logger) {
            logger.critical(category, message)
        } else {
            console.error("[CRITICAL] [" + category + "] " + message)
        }
    }

    function fatal(category, message) {
        if (logger) {
            logger.fatal(category, message)
        } else {
            console.error("[FATAL] [" + category + "] " + message)
        }
    }

    function debugMsg(message) {
        debug("QML", message)
    }

    function infoMsg(message) {
        info("QML", message)
    }

    function warningMsg(message) {
        warning("QML", message)
    }

    function criticalMsg(message) {
        critical("QML", message)
    }

    function fatalMsg(message) {
        fatal("QML", message)
    }
}