#ifndef FDT_CONFIG_H
#define FDT_CONFIG_H

/**
 * @brief FDT_BEGIN
 * *The beginning of each node's representation.
 * *It shall be followed by a unit name and shall include 
 * *unit-address if any. and the node name is followed by zeroed
 * *padding bytes, and the next token will be any token except 
 * *FDT_END
 */
#define FDT_BEGIN_NODE      0x00000001
/**
 * @brief FDT_END
 * *The end of node's representation, and the followed token
 * *shall be any token except FDT_PROT
 */
#define FDT_END_NODE        0x00000002
/**
 * @brief FDT_PROT
 * *Mark the beginning of the representation of one property
 * *in the devicetree. zeroed padding to align to a 4-byte bundary
 */
#define FDT_PROP            0x00000003

/**
 * @brief FDT_NOP
 * * Will be ignored by parser
 */
#define FDT_NOP             0x00000004

/**
 * @brief FDT_END
 * *The FDT_END token marks the end of the structure block.
 */
#define FDT_END             0x00000009


















#endif