# Add / Modify device profile for device provisioning

- [Add / Modify device profile for device provisioning](#add--modify-device-profile-for-device-provisioning)
  - [Provison strateay - Allow to create new devices](#provison-strateay---allow-to-create-new-devices)
    - [Add device profile and allow to create new devices](#add-device-profile-and-allow-to-create-new-devices)
    - [Modify device profile to allow to create new devices](#modify-device-profile-to-allow-to-create-new-devices)
  - [Provison strateay - Check for pre-provisioned devices](#provison-strateay---check-for-pre-provisioned-devices)
    - [Add device profile and check for pre-provisioned devices](#add-device-profile-and-check-for-pre-provisioned-devices)
    - [Modify device profile to check for pre-provisioned devices](#modify-device-profile-to-check-for-pre-provisioned-devices)


## Provison strateay - Allow to create new devices

### Add device profile and allow to create new devices

1. `Login in ThingsBoard CE/PE` --> `Devices` -->`Device profiles` --> `+`  --> `Create new device profiles` --> Input **Device Profile Name** --> `Device provisioning Optional`.

    ![image](images/add_or_modify_device_profile_for_device_provisioning/add-device-profile-1.png)

1. `Allow to create new devices` --> Input `Provision device key` & `Provision device secret`, copy to a safe place --> `Add`.

    ![image](images/add_or_modify_device_profile_for_device_provisioning/add-device-profile-and-allow-to-create-new-devices-2.png)

1. Now my device profile should be listed first, since the table sort devices using the time of the creation by default..

    ![image](images/add_or_modify_device_profile_for_device_provisioning/add-device-profile-3.png)

### Modify device profile to allow to create new devices

1. `Login in ThingsBoard CE/PE` --> `Device profiles` --> click *my testing device profiles*  --> `Device provisioning` --> `Toggle edit mode` (red icon).

    ![image](images/add_or_modify_device_profile_for_device_provisioning/modify-device-profile-1.png)

1. `Allow to create new devices` --> Input `Provision device key` & `Provision device secret`, copy to a safe place --> `Apply changes` (red icon).

    ![image](images/add_or_modify_device_profile_for_device_provisioning/modify-device-profile-to-allow-to-create-new-devices-2.png)

## Provison strateay - Check for pre-provisioned devices

### Add device profile and check for pre-provisioned devices

1. `Login in ThingsBoard CE/PE` --> `Devices` -->`Device profiles` --> `+`  --> `Create new device profiles` --> Input **Device Profile Name** --> `Device provisioning Optional`.

    ![image](images/add_or_modify_device_profile_for_device_provisioning/add-device-profile-1.png)

1. `Check for pre-provisioned devices` --> Input `Provision device key` & `Provision device secret`, copy to a safe place --> `Add`.

    ![image](images/add_or_modify_device_profile_for_device_provisioning/add-device-profile-and-check-for-pre-provisioned-devices-2.png)

1. Now my device profile should be listed first, since the table sort devices using the time of the creation by default..

    ![image](images/add_or_modify_device_profile_for_device_provisioning/add-device-profile-3.png)


### Modify device profile to check for pre-provisioned devices

1. `Login in ThingsBoard CE/PE` --> `Device profiles` --> click *my testing device profiles*  --> `Device provisioning` --> `Toggle edit mode` (red icon).

    ![image](images/add_or_modify_device_profile_for_device_provisioning/modify-device-profile-1.png)

1. `Check for pre-provisoned devices` --> Input `Provision device key` & `Provision device secret`, copy to a safe place --> `Apply changes` (red icon).

    ![image](images/add_or_modify_device_profile_for_device_provisioning/modify-device-profile-to-check-for-pre-provisioned-devices-2.png)