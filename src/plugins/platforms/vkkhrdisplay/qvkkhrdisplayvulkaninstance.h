/****************************************************************************
**
** Copyright (C) 2021 The Qt Company Ltd.
** Contact: https://www.qt.io/licensing/
**
** This file is part of the plugins of the Qt Toolkit.
**
** $QT_BEGIN_LICENSE:LGPL$
** Commercial License Usage
** Licensees holding valid commercial Qt licenses may use this file in
** accordance with the commercial license agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and The Qt Company. For licensing terms
** and conditions see https://www.qt.io/terms-conditions. For further
** information use the contact form at https://www.qt.io/contact-us.
**
** GNU Lesser General Public License Usage
** Alternatively, this file may be used under the terms of the GNU Lesser
** General Public License version 3 as published by the Free Software
** Foundation and appearing in the file LICENSE.LGPL3 included in the
** packaging of this file. Please review the following information to
** ensure the GNU Lesser General Public License version 3 requirements
** will be met: https://www.gnu.org/licenses/lgpl-3.0.html.
**
** GNU General Public License Usage
** Alternatively, this file may be used under the terms of the GNU
** General Public License version 2.0 or (at your option) the GNU General
** Public license version 3 or any later version approved by the KDE Free
** Qt Foundation. The licenses are as published by the Free Software
** Foundation and appearing in the file LICENSE.GPL2 and LICENSE.GPL3
** included in the packaging of this file. Please review the following
** information to ensure the GNU General Public License requirements will
** be met: https://www.gnu.org/licenses/gpl-2.0.html and
** https://www.gnu.org/licenses/gpl-3.0.html.
**
** $QT_END_LICENSE$
**
****************************************************************************/

#ifndef QVKKHRDISPLAYVULKANINSTANCE_H
#define QVKKHRDISPLAYVULKANINSTANCE_H

//
//  W A R N I N G
//  -------------
//
// This file is not part of the Qt API.  It exists purely as an
// implementation detail.  This header file may change from version to
// version without notice, or even be removed.
//
// We mean it.
//

#include <QtGui/private/qbasicvulkanplatforminstance_p.h>
#include <QtCore/qsize.h>

QT_BEGIN_NAMESPACE

class QVkKhrDisplayVulkanInstance : public QBasicPlatformVulkanInstance
{
public:
    QVkKhrDisplayVulkanInstance(QVulkanInstance *instance);

    void createOrAdoptInstance() override;
    bool supportsPresent(VkPhysicalDevice physicalDevice, uint32_t queueFamilyIndex, QWindow *window) override;
    void presentAboutToBeQueued(QWindow *window) override;

    VkSurfaceKHR createSurface(QWindow *window);

    QSize displaySize() const { return QSize(int(m_width), int(m_height)); }

    using CreatedCallback = void (*)(QVkKhrDisplayVulkanInstance *, void *);
    void setCreatedCallback(CreatedCallback callback, void *userData) {
        m_createdCallback = callback;
        m_createdCallbackUserData = userData;
    }

private:
    bool chooseDisplay();

    QVulkanInstance *m_instance;
    VkPhysicalDevice m_physDev = VK_NULL_HANDLE;
    PFN_vkEnumeratePhysicalDevices m_enumeratePhysicalDevices = nullptr;
    PFN_vkGetPhysicalDeviceSurfaceSupportKHR m_getPhysicalDeviceSurfaceSupportKHR = nullptr;
#if VK_KHR_display
    PFN_vkGetPhysicalDeviceDisplayPropertiesKHR m_getPhysicalDeviceDisplayPropertiesKHR = nullptr;
    PFN_vkGetDisplayModePropertiesKHR m_getDisplayModePropertiesKHR = nullptr;
    PFN_vkGetPhysicalDeviceDisplayPlanePropertiesKHR m_getPhysicalDeviceDisplayPlanePropertiesKHR = nullptr;
    PFN_vkGetDisplayPlaneSupportedDisplaysKHR m_getDisplayPlaneSupportedDisplaysKHR = nullptr;
    PFN_vkGetDisplayPlaneCapabilitiesKHR m_getDisplayPlaneCapabilitiesKHR = nullptr;
    PFN_vkCreateDisplayPlaneSurfaceKHR m_createDisplayPlaneSurfaceKHR = nullptr;
#endif

    CreatedCallback m_createdCallback = nullptr;
    void *m_createdCallbackUserData = nullptr;
    VkDisplayKHR m_display = VK_NULL_HANDLE;
    VkDisplayModeKHR m_displayMode = VK_NULL_HANDLE;
    uint32_t m_width = 0;
    uint32_t m_height = 0;
    uint32_t m_planeIndex = UINT_MAX;
    uint32_t m_planeStackIndex = UINT_MAX;
};

QT_END_NAMESPACE

#endif
