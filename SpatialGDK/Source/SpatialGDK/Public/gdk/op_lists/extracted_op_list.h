#ifndef GDK_EXTRACTED_OP_LIST_H
#define GDK_EXTRACTED_OP_LIST_H
#include "gdk/op_lists/op_list.h"
#include <vector>

namespace gdk {

// todo - consider building an op list by deep copying the contents of each op in the host list.
/** An op list that is created by extracting ops from other op lists. */
class ExtractedOpList : public AbstractOpList {
public:
  size_t GetCount() const override;
  Worker_Op& operator[](size_t index) override;

  /**
   * Shallow copies an op into the extracted op list and zeros out the op in the donor list.
   * The donor OpList must still exist for the ExtractedOpList to be valid.
   */
  void AddOp(OpList* opList, size_t index);

private:
  std::vector<Worker_Op> ops;
};

}  // namespace gdk
#endif  // GDK_EXTRACTED_OP_LIST_H