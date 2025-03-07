#include "standards/iso3779.h"

iso3779decoded * iso3779_vin_new() {
    iso3779decoded * vinDecoded = (iso3779decoded*)malloc(sizeof(iso3779decoded));
    vinDecoded->wmi.country = null;
    return vinDecoded;
}
void iso3779_vin_free(iso3779decoded *vin) {
    if ( vin->wmi.country != null ) {
        free(vin->wmi.country);
        vin->wmi.country = null;
    }
    free(vin);
}
char * iso3779decode_region_from(final Buffer *vin_raw) {
    final char * vin = buffer_to_ascii(vin_raw);
    if ( buffer_alphabet_compare(vin,"A","H") ) {
        return strdup("Africa");
    }
    if ( buffer_alphabet_compare(vin,"J","R") ) {
        return strdup("Asia");
    }
    if ( buffer_alphabet_compare(vin,"S","Z") ) {
        return strdup("Europe");
    }
    if ( buffer_alphabet_compare(vin,"1","5") ) {
        return strdup("North America");
    }
    if ( buffer_alphabet_compare(vin,"6","7") ) {
        return strdup("Oceania");
    }
    if ( buffer_alphabet_compare(vin,"8","9") ) {
        return strdup("South America");
    }
    return strdup("Unknown");
}
char * iso3779decode_country_from(final Buffer *vin_raw) {
    final char * vin = buffer_to_ascii(vin_raw);
    if ( buffer_alphabet_compare(vin,"AA","AH") ) {
        return strdup("South Africa");
    }
    if ( buffer_alphabet_compare(vin,"AJ","AN") ) {
        return strdup("Ivory Coast");
    }
    if ( buffer_alphabet_compare(vin,"AP","A0") ) {
        return strdup("Unassigned");
    }
    if ( buffer_alphabet_compare(vin,"BA","BE") ) {
        return strdup("Angola");
    }
    if ( buffer_alphabet_compare(vin,"BF","BK") ) {
        return strdup("Kenya");
    }
    if ( buffer_alphabet_compare(vin,"BL","BR") ) {
        return strdup("Tanzania");
    }
    if ( buffer_alphabet_compare(vin,"BS","B0") ) {
        return strdup("Unassigned");
    }
    if ( buffer_alphabet_compare(vin,"CA","CE") ) {
        return strdup("Benin");
    }
    if ( buffer_alphabet_compare(vin,"CF","CK") ) {
        return strdup("Madagascar");
    }
    if ( buffer_alphabet_compare(vin,"CL","CR") ) {
        return strdup("Tunisia");
    }
    if ( buffer_alphabet_compare(vin,"CS","C0") ) {
        return strdup("Unassigned");
    }
    if ( buffer_alphabet_compare(vin,"DA","DE") ) {
        return strdup("Egypt");
    }
    if ( buffer_alphabet_compare(vin,"DF","DK") ) {
        return strdup("Morocco");
    }
    if ( buffer_alphabet_compare(vin,"DL","DR") ) {
        return strdup("Zambia");
    }
    if ( buffer_alphabet_compare(vin,"DS","D0") ) {
        return strdup("Unassigned");
    }
    if ( buffer_alphabet_compare(vin,"EA","EE") ) {
        return strdup("Ethiopia");
    }
    if ( buffer_alphabet_compare(vin,"EF","EK") ) {
        return strdup("Mozambique");
    }
    if ( buffer_alphabet_compare(vin,"EL","E0") ) {
        return strdup("Unassigned");
    }
    if ( buffer_alphabet_compare(vin,"FA","FE") ) {
        return strdup("Ghana");
    }
    if ( buffer_alphabet_compare(vin,"FF","FK") ) {
        return strdup("Nigeria");
    }
    if ( buffer_alphabet_compare(vin,"FF","FK") ) {
        return strdup("Madagascar");
    }
    if ( buffer_alphabet_compare(vin,"FL","F0") ) {
        return strdup("Unassigned");
    }
    if ( buffer_alphabet_compare(vin,"GA","G0") ) {
        return strdup("Unassigned");
    }
    if ( buffer_alphabet_compare(vin,"HA","H0") ) {
        return strdup("Unassigned");
    }
    if ( buffer_alphabet_compare(vin,"JA","J0") ) {
        return strdup("Japan");
    }
    if ( buffer_alphabet_compare(vin,"KA","KE") ) {
        return strdup("Sri Lanka");
    }
    if ( buffer_alphabet_compare(vin,"KF","KK") ) {
        return strdup("Israel");
    }
    if ( buffer_alphabet_compare(vin,"KL","KR") ) {
        return strdup("South Korea");
    }
    if ( buffer_alphabet_compare(vin,"KS","K0") ) {
        return strdup("Unassigned");
    }
    if ( buffer_alphabet_compare(vin,"LA","L0") ) {
        return strdup("China");
    }
    if ( buffer_alphabet_compare(vin,"MA","ME") ) {
        return strdup("India");
    }
    if ( buffer_alphabet_compare(vin,"MF","MK") ) {
        return strdup("Indonesia");
    }
    if ( buffer_alphabet_compare(vin,"ML","MR") ) {
        return strdup("Thailand");
    }
    if ( buffer_alphabet_compare(vin,"MS","M0") ) {
        return strdup("Unassigned");
    }
    if ( buffer_alphabet_compare(vin,"NF","NK") ) {
        return strdup("Pakistan");
    }
    if ( buffer_alphabet_compare(vin,"NL","NR") ) {
        return strdup("Turkey");
    }
    if ( buffer_alphabet_compare(vin,"NS","N0") ) {
        return strdup("Unassigned");
    }
    if ( buffer_alphabet_compare(vin,"PA","PE") ) {
        return strdup("Philippines");
    }
    if ( buffer_alphabet_compare(vin,"PF","PK") ) {
        return strdup("Singapore");
    }
    if ( buffer_alphabet_compare(vin,"PL","PR") ) {
        return strdup("Malaysia");
    }
    if ( buffer_alphabet_compare(vin,"PS","P0") ) {
        return strdup("Unassigned");
    }
    if ( buffer_alphabet_compare(vin,"RA","RE") ) {
        return strdup("United Arab Emirates");
    }
    if ( buffer_alphabet_compare(vin,"RF","RK") ) {
        return strdup("Taiwan");
    }
    if ( buffer_alphabet_compare(vin,"RL","RR") ) {
        return strdup("Vietnam");
    }
    if ( buffer_alphabet_compare(vin,"RS","R0") ) {
        return strdup("Unassigned");
    }
    if ( buffer_alphabet_compare(vin,"SA","SM") ) {
        return strdup("Great Britain");
    }
    if ( buffer_alphabet_compare(vin,"SN","ST") ) {
        return strdup("Germany");
    }
    if ( buffer_alphabet_compare(vin,"SU","SZ") ) {
        return strdup("Poland");
    }
    if ( buffer_alphabet_compare(vin,"S1","S0") ) {
        return strdup("Unassigned");
    }
    if ( buffer_alphabet_compare(vin,"TA","TH") ) {
        return strdup("Switzerland");
    }
    if ( buffer_alphabet_compare(vin,"TJ","TP") ) {
        return strdup("Czech Republic");
    }
    if ( buffer_alphabet_compare(vin,"TR","TV") ) {
        return strdup("Hungary");
    }
    if ( buffer_alphabet_compare(vin,"TW","T1") ) {
        return strdup("Portugal");
    }
    if ( buffer_alphabet_compare(vin,"T2","T0") ) {
        return strdup("Unassigned");
    }
    if ( buffer_alphabet_compare(vin,"UA","UG") ) {
        return strdup("Unassigned");
    }
    if ( buffer_alphabet_compare(vin,"UH","UM") ) {
        return strdup("Denmark");
    }
    if ( buffer_alphabet_compare(vin,"UN","UT") ) {
        return strdup("Ireland");
    }
    if ( buffer_alphabet_compare(vin,"UU","UZ") ) {
        return strdup("Romania");
    }
    if ( buffer_alphabet_compare(vin,"U1","U4") ) {
        return strdup("Unassigned");
    }
    if ( buffer_alphabet_compare(vin,"U5","U7") ) {
        return strdup("Slovakia");
    }
    if ( buffer_alphabet_compare(vin,"U8","U0") ) {
        return strdup("Unassigned");
    }
    if ( buffer_alphabet_compare(vin,"VA","VE") ) {
        return strdup("Austria");
    }
    if ( buffer_alphabet_compare(vin,"VF","VR") ) {
        return strdup("France");
    }
    if ( buffer_alphabet_compare(vin,"VS","VW") ) {
        return strdup("Spain");
    }
    if ( buffer_alphabet_compare(vin,"VX","V2") ) {
        return strdup("Yugoslavia");
    }
    if ( buffer_alphabet_compare(vin,"V3","V5") ) {
        return strdup("Croatia");
    }
    if ( buffer_alphabet_compare(vin,"V6","V0") ) {
        return strdup("Estonia");
    }
    if ( buffer_alphabet_compare(vin,"WA","W0") ) {
        return strdup("Germany");
    }
    if ( buffer_alphabet_compare(vin,"XA","XE") ) {
        return strdup("Bulgaria");
    }
    if ( buffer_alphabet_compare(vin,"XF","XK") ) {
        return strdup("Greece");
    }
    if ( buffer_alphabet_compare(vin,"XL","XR") ) {
        return strdup("Netherlands");
    }
    if ( buffer_alphabet_compare(vin,"XS","XW") ) {
        return strdup("Russia");
    }
    if ( buffer_alphabet_compare(vin,"XX","X2") ) {
        return strdup("Luxembourg");
    }
    if ( buffer_alphabet_compare(vin,"X3","X0") ) {
        return strdup("Russia");
    }
    if ( buffer_alphabet_compare(vin,"YA","YE") ) {
        return strdup("Belgium");
    }
    if ( buffer_alphabet_compare(vin,"YF","YK") ) {
        return strdup("Finland");
    }
    if ( buffer_alphabet_compare(vin,"YL","YR") ) {
        return strdup("Malta");
    }
    if ( buffer_alphabet_compare(vin,"YS","YW") ) {
        return strdup("Sweden");
    }
    if ( buffer_alphabet_compare(vin,"YX","Y2") ) {
        return strdup("Norway");
    }
    if ( buffer_alphabet_compare(vin,"Y3","Y5") ) {
        return strdup("Belarus");
    }
    if ( buffer_alphabet_compare(vin,"Y6","Y0") ) {
        return strdup("Ukraine");
    }
    if ( buffer_alphabet_compare(vin,"ZA","ZR") ) {
        return strdup("Italy");
    }
    if ( buffer_alphabet_compare(vin,"ZS","ZW") ) {
        return strdup("Unassigned");
    }
    if ( buffer_alphabet_compare(vin,"ZX","Z2") ) {
        return strdup("Slovenia");
    }
    if ( buffer_alphabet_compare(vin,"Z3","Z5") ) {
        return strdup("Lithuania");
    }
    if ( buffer_alphabet_compare(vin,"Z6","Z0") ) {
        return strdup("Unassigned");
    }
    if ( buffer_alphabet_compare(vin,"1A","10") ) {
        return strdup("United States");
    }
    if ( buffer_alphabet_compare(vin,"2A","20") ) {
        return strdup("Canada");
    }
    if ( buffer_alphabet_compare(vin,"3A","3W") ) {
        return strdup("Mexico");
    }
    if ( buffer_alphabet_compare(vin,"3X","37") ) {
        return strdup("Costa Rica");
    }
    if ( buffer_alphabet_compare(vin,"38","30") ) {
        return strdup("Unassigned");
    }
    if ( buffer_alphabet_compare(vin,"4A","40") ) {
        return strdup("United States");
    }
    if ( buffer_alphabet_compare(vin,"5A","50") ) {
        return strdup("United States");
    }
    if ( buffer_alphabet_compare(vin,"6A","6W") ) {
        return strdup("Australia");
    }
    if ( buffer_alphabet_compare(vin,"6X","60") ) {
        return strdup("Unassigned");
    }
    if ( buffer_alphabet_compare(vin,"7A","7E") ) {
        return strdup("New Zealand");
    }
    if ( buffer_alphabet_compare(vin,"7F","70") ) {
        return strdup("Unassigned");
    }
    if ( buffer_alphabet_compare(vin,"8A","8E") ) {
        return strdup("Argentina");
    }
    if ( buffer_alphabet_compare(vin,"8F","8K") ) {
        return strdup("Chile");
    }
    if ( buffer_alphabet_compare(vin,"8L","8R") ) {
        return strdup("Ecuador");
    }
    if ( buffer_alphabet_compare(vin,"8S","8W") ) {
        return strdup("Peru");
    }
    if ( buffer_alphabet_compare(vin,"8X","82") ) {
        return strdup("Venezuela");
    }
    if ( buffer_alphabet_compare(vin,"83","80") ) {
        return strdup("Unassigned");
    }
    if ( buffer_alphabet_compare(vin,"9A","9E") ) {
        return strdup("Brazil");
    }
    if ( buffer_alphabet_compare(vin,"9F","9K") ) {
        return strdup("Colombia");
    }
    if ( buffer_alphabet_compare(vin,"9L","9R") ) {
        return strdup("Paraguay");
    }
    if ( buffer_alphabet_compare(vin,"9S","9W") ) {
        return strdup("Uruguay");
    }
    if ( buffer_alphabet_compare(vin,"9X","92") ) {
        return strdup("Trinidad & Tobago");
    }
    if ( buffer_alphabet_compare(vin,"93","99") ) {
        return strdup("Brazil");
    }
    return strdup("Unassigned");
}
iso3779decoded* iso3779decode_from(final Buffer *vin) {
    iso3779decoded * vinDecoded = iso3779_vin_new();
    vinDecoded->wmi.country = iso3779decode_country_from(vin);
    return vinDecoded;
}
