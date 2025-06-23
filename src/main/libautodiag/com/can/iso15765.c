#include "libautodiag/com/can/iso15765.h"

Iso15765Conversation* iso15765_init_conversation(int expected_bytes) {
    Iso15765Conversation* conv = (Iso15765Conversation*)malloc(sizeof(Iso15765Conversation));
    conv->current_sn = 0;
    conv->current_data_length = 0;
    conv->remaining_data_bytes_to_receive = expected_bytes;
    conv->data = buffer_new();
    buffer_ensure_capacity(conv->data,expected_bytes);
    conv->data->size = conv->data->size_allocated;
    conv->ecu = null;
    return conv;
}

void iso15765_conversation_free(Iso15765Conversation *conv) {
    if ( conv != null ) {
        buffer_free(conv->data);
        conv->remaining_data_bytes_to_receive = 0;
        conv->ecu = null;
        free(conv);
    }
}

bool list_Iso15765Conversation_find_comparator(Iso15765Conversation *conv, ECU* ecu) {
    return conv->ecu == ecu;
}
LIST_SRC_FIND(Iso15765Conversation,
                ECU*,list_Iso15765Conversation_find_comparator
)
LIST_SRC(Iso15765Conversation)
