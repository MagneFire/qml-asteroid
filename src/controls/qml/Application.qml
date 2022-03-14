/*
 * Copyright (C) 2016 Florent Revest <revestflo@gmail.com>
 *               2015 Tim Süberkrüb <tim.sueberkrueb@web.de>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as
 * published by the Free Software Foundation, either version 2.1 of the
 * License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program. If not, see <http://www.gnu.org/licenses/>.
 */

import QtQuick 2.9
import org.asteroid.controls 1.0
import org.asteroid.utils 1.0
import Nemo.Configuration 1.0

Application_p {
    anchors.fill: parent

    property alias outerColor: fm.outerColor
    property alias centerColor: fm.centerColor

    function animIndicators() {
        rightIndicator.animate();
        leftIndicator.animate();
        topIndicator.animate();
        bottomIndicator.animate();
    }

    ConfigurationValue {
        id: fpsCounter
        key: "/org/asteroidos/settings/fps-counter"
        defaultValue: false
    }

    FlatMesh {
        id: fm
        anchors.fill: parent
    }

    FPSText {
        id: fps
        visible: fpsCounter.value
        enabled: visible
        anchors.fill: parent
        z: 10
        Label {
            anchors.top: parent.top
            anchors.topMargin: Dims.h(2)
            anchors.horizontalCenter: parent.horizontalCenter
            text: fps.fps
            font.styleName: "Bold"
            style: Text.Outline
            styleColor: fps.fps > 50 ? "green" : "red"
        }
    }

    property alias rightIndicVisible:  rightIndicator.visible
    property alias leftIndicVisible:   leftIndicator.visible
    property alias topIndicVisible:    topIndicator.visible
    property alias bottomIndicVisible: bottomIndicator.visible

    Indicator {
        id: rightIndicator
        edge: Qt.RightEdge
        visible: false
        z: 10
        anchors.verticalCenterOffset: DeviceInfo.flatTireHeight/2
    }

    Indicator {
        id: leftIndicator
        edge: Qt.LeftEdge
        visible: true
        z: 10
        anchors.verticalCenterOffset: DeviceInfo.flatTireHeight/2
    }

    Indicator {
        id: topIndicator
        edge: Qt.TopEdge
        visible: true
        z: 10
    }

    Indicator {
        id: bottomIndicator
        edge: Qt.BottomEdge
        visible: false
        z: 10
    }
}
