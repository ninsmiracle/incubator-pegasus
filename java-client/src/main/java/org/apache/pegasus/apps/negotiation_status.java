/*
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 */
/**
 * Autogenerated by Thrift Compiler (0.11.0)
 *
 * <p>DO NOT EDIT UNLESS YOU ARE SURE THAT YOU KNOW WHAT YOU ARE DOING
 *
 * @generated
 */
package org.apache.pegasus.apps;

public enum negotiation_status implements org.apache.thrift.TEnum {
  INVALID(0),
  SASL_LIST_MECHANISMS(1),
  SASL_LIST_MECHANISMS_RESP(2),
  SASL_SELECT_MECHANISMS(3),
  SASL_SELECT_MECHANISMS_RESP(4),
  SASL_INITIATE(5),
  SASL_CHALLENGE(6),
  SASL_CHALLENGE_RESP(7),
  SASL_SUCC(8),
  SASL_AUTH_DISABLE(9),
  SASL_AUTH_FAIL(10);

  private final int value;

  private negotiation_status(int value) {
    this.value = value;
  }

  /** Get the integer value of this enum value, as defined in the Thrift IDL. */
  public int getValue() {
    return value;
  }

  /**
   * Find a the enum type by its integer value, as defined in the Thrift IDL.
   *
   * @return null if the value is not found.
   */
  public static negotiation_status findByValue(int value) {
    switch (value) {
      case 0:
        return INVALID;
      case 1:
        return SASL_LIST_MECHANISMS;
      case 2:
        return SASL_LIST_MECHANISMS_RESP;
      case 3:
        return SASL_SELECT_MECHANISMS;
      case 4:
        return SASL_SELECT_MECHANISMS_RESP;
      case 5:
        return SASL_INITIATE;
      case 6:
        return SASL_CHALLENGE;
      case 7:
        return SASL_CHALLENGE_RESP;
      case 8:
        return SASL_SUCC;
      case 9:
        return SASL_AUTH_DISABLE;
      case 10:
        return SASL_AUTH_FAIL;
      default:
        return null;
    }
  }
}