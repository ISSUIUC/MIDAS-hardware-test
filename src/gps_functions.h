#pragma once

enum NMEAPacket {
    GGA, // GPS Fixed data
    GLL, // Geographic position
    GSA, // GNSS DOP and active satellites
    GSV, // Satellites in view
    MSS, // MSK Reciever Signal
    RMC, // Recommended minimum specific GNSS data
    VTG, // Ground speed
    ZDA // SiRF data
};

