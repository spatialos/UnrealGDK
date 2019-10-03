#include "gdk/op_lists/extracted_op_list.h"

namespace gdk {
  
size_t ExtractedOpList::GetCount() const {
  return ops.size();
}

Worker_Op& ExtractedOpList::operator[](size_t index) {
  return ops[index];
}

void ExtractedOpList::AddOp(OpList* opList, size_t index) {
  auto& op = (*opList)[index];
  ops.emplace_back(op);
  op.op_type = 0;
}

}  // namespace gdk