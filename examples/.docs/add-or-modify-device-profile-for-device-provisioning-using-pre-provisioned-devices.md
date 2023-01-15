# Add / Modify a device profile for device provisioning - Check for pre-provisioned devices

## Add a new device profile and check for pre-provisioned devices

1. `Login in ThingsBoard CE/PE as tenant` --> `Devices` -->`Device profiles` --> `+`  --> `Create new device profiles` --> Input *Device Profile Name* --> `Device provisioning Optional`.

    ![image](images/add-or-modify-device-profile-for-device-provisioning-using-pre-provisioned-devices/add-device-profile-1.png)

1. `Check for pre-provisioned devices` --> Input `Provision device key` & `Provision device secret`, copy to a safe place --> `Add`.

    ![image](images/add-or-modify-device-profile-for-device-provisioning-using-pre-provisioned-devices/add-device-profile-2.png)

1. Now my device profile should be listed first, since the table sort devices using the time of the creation by default.

    ![image](images/add-or-modify-device-profile-for-device-provisioning-using-pre-provisioned-devices/add-device-profile-3.png)


## Modify a device profile to check for pre-provisioned devices

1. `Login in ThingsBoard CE/PE as tenant` --> `Device profiles` --> click *my device profile*  --> `Device provisioning` --> `Toggle edit mode` (red icon).

    ![image](images/add-or-modify-device-profile-for-device-provisioning-using-pre-provisioned-devices/modify-device-profile-1.png)

1. `Check for pre-provisoned devices` --> Input `Provision device key` & `Provision device secret`, copy to a safe place --> `Apply changes` (red icon).

    ![image](images/add-or-modify-device-profile-for-device-provisioning-using-pre-provisioned-devices/modify-device-profile-2.png)
